#include "BufferPool.h"
#include "interface/internal/IShaderInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IBufferPoolInternal.h"
#include "VulkanFactories.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "shaders/VulkanShaderManager.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/ISpecConstLayout.h>
#include <TF2Vulkan/Util/AutoInit.h>
#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <unordered_map>
#include <unordered_set>

using namespace TF2Vulkan;

namespace
{
	struct SpecConstLayout final : ISpecConstLayout
	{
		SpecConstLayout(const SpecConstLayoutCreateInfo& ci);

#ifndef __INTELLISENSE__
		std::strong_ordering operator<=>(const SpecConstLayout& other) const noexcept
		{
			if (auto result = m_CreateInfo <=> other.m_CreateInfo; std::is_neq(result))
				return result;
			if (auto result = m_BufferSize <=> other.m_BufferSize; std::is_neq(result))
				return result;

			return std::strong_ordering::equal;
		}
#endif

		size_t GetBufferSize() const override { return m_BufferSize; }
		const SpecConstLayoutCreateInfo& GetCreateInfo() const override { return m_CreateInfo; }

		auto begin() const { return m_CreateInfo.begin(); }
		auto end() const { return m_CreateInfo.end(); }

	private:
		const SpecConstLayoutCreateInfo m_CreateInfo;
		size_t m_BufferSize;
	};

	struct ShaderGroupKey final
	{
		DEFAULT_STRONG_EQUALITY_OPERATOR(ShaderGroupKey);
		CUtlSymbolDbg m_Name;
		const ISpecConstLayout* m_Layout = nullptr;;
	};
}

STD_HASH_DEFINITION(ShaderGroupKey,
	v.m_Name,
	v.m_Layout
);


template<typename T> static size_t HashSpecConstLayout(const T& layout)
{
	size_t count;
	const auto* first = layout.GetEntries(count);
	return Util::hash_range(first, first + count);
}

template<> struct ::std::hash<ISpecConstLayout>
{
	size_t operator()(const ISpecConstLayout& layout) const { return HashSpecConstLayout(layout); }
};
template<> struct ::std::hash<SpecConstLayout>
{
	size_t operator()(const SpecConstLayout& layout) const { return HashSpecConstLayout(layout); }
};

namespace
{
	struct ShaderGroup;
	struct ShaderInstance final : IShaderInstanceInternal
	{
		ShaderInstance(const ShaderGroup& group, const void* specConstBuffer);
		ShaderInstance(ShaderInstance&&) = default;
		ShaderInstance& operator=(ShaderInstance&&) = default;
		~ShaderInstance() { assert(m_DeletionAllowed); }

		// Inherited via IShaderInstanceInternal
		const ShaderGroup& GetGroupInternal() const;
		const IShaderGroupInternal& GetGroup() const override;
		const void* GetSpecConstBuffer() const override { return m_SpecConstBuffer.get(); }
		void GetSpecializationInfo(vk::SpecializationInfo& info) const override;

		void DisallowDeletion() { m_DeletionAllowed = false; }

	private:
		bool m_DeletionAllowed = true;
		const ShaderGroup* m_Group = nullptr;
		std::unique_ptr<const std::byte[]> m_SpecConstBuffer;
	};

	struct ShaderGroup final : IShaderGroupInternal
	{
		ShaderGroup(ShaderType type, const IVulkanShader& shader, const ISpecConstLayout& layout);

		// Inherited via IShaderGroupInternal
		const char* GetName() const override { return GetVulkanShader().GetName().String(); }
		const ISpecConstLayout& GetSpecConstLayout() const override { return *m_SpecConstLayout; }
		ShaderType GetShaderType() const override { return m_Type; }
		IShaderInstance& FindOrCreateInstance(const void* specConstBuf, size_t specConstBufSize) override;
		const IVulkanShader& GetVulkanShader() const override { return *m_VulkanShader; }
		const vk::SpecializationMapEntry* GetMapEntries(uint32_t& count) const;
		UniformBufferIndex FindUniformBuffer(const std::string_view& name) const override;

	private:
		ShaderType m_Type;
		const IVulkanShader* m_VulkanShader = nullptr;
		const ISpecConstLayout* m_SpecConstLayout = nullptr;
		std::vector<vk::SpecializationMapEntry> m_VkEntries;

		struct Hasher
		{
			constexpr Hasher(size_t size) : m_Size(size) {}
			size_t operator()(const std::byte* data) const;
			size_t m_Size;
		};
		struct KeyEqual
		{
			constexpr KeyEqual(size_t size) : m_Size(size) {}
			size_t operator()(const std::byte* lhs, const std::byte* rhs) const { return memcmp(lhs, rhs, m_Size) == 0; }
			size_t m_Size;
		};
		std::unordered_map<const std::byte*, ShaderInstance, Hasher, KeyEqual> m_Instances;
	};

	class ShaderNextFactory final : public IShaderNextFactory, Util::IAutoInit<IShaderDeviceInternal>
	{
	public:
		ShaderNextFactory();

		void AutoInit() override;

		IBufferPool& GetUniformBufferPool() override { return m_UniformBufferPool.value(); }

		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const ISpecConstLayout* layout) override;
		const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutCreateInfo& ci) override;

	private:
		std::array<std::unordered_map<ShaderGroupKey, ShaderGroup>, 2> m_ShaderInstanceGroups;
		const ISpecConstLayout* m_EmptySCLayout;
		std::unordered_set<SpecConstLayout> m_SpecConstLayouts;
		std::optional<BufferPoolContiguous> m_UniformBufferPool;
	};
}

static ShaderNextFactory s_ShaderInstMgr;
IShaderNextFactory* TF2Vulkan::g_ShaderFactory = &s_ShaderInstMgr;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShaderNextFactory, IShaderNextFactory, SHADERNEXTFACTORY_INTERFACE_VERSION, s_ShaderInstMgr);

ShaderNextFactory::ShaderNextFactory()
{
	m_EmptySCLayout = &*m_SpecConstLayouts.emplace(SpecConstLayoutCreateInfo{}).first;
}

void ShaderNextFactory::AutoInit()
{
	m_UniformBufferPool.emplace(1024 * 1024 * 8, vk::BufferUsageFlagBits::eUniformBuffer);
}

SpecConstLayout::SpecConstLayout(const SpecConstLayoutCreateInfo& ci) :
	m_CreateInfo(ci)
{
	assert(!!m_CreateInfo.m_Entries == !!m_CreateInfo.m_EntryCount);

	// Make sure there's no padding and they're in order
	size_t nextOffset = 0;
	for (size_t i = 0; i < m_CreateInfo.m_EntryCount; i++)
	{
		assert(m_CreateInfo.m_Entries[i].m_Offset == nextOffset);
		nextOffset += 4;
	}

	m_BufferSize = nextOffset;
}

ShaderInstance::ShaderInstance(const ShaderGroup& group, const void* specConstBuffer) :
	m_Group(&group)
{
	const auto bufSize = group.GetSpecConstLayout().GetBufferSize();
	if (bufSize)
	{
		auto buf = std::make_unique<std::byte[]>(bufSize);
		memcpy(buf.get(), specConstBuffer, bufSize);
		m_SpecConstBuffer = std::move(buf);
	}
}

void ShaderInstance::GetSpecializationInfo(vk::SpecializationInfo& info) const
{
	// TODO: Warning about providing data for spec constants that aren't referenced?

	info.pMapEntries = GetGroupInternal().GetMapEntries(info.mapEntryCount);
	if (info.pMapEntries)
	{
		info.dataSize = m_Group->GetSpecConstLayout().GetBufferSize();
		info.pData = m_SpecConstBuffer.get();
	}

	assert(!info.mapEntryCount == !info.pMapEntries);
	assert(!info.dataSize == !info.pData);
	assert(!info.dataSize == !info.mapEntryCount);
}

const ShaderGroup& ShaderInstance::GetGroupInternal() const
{
	return *m_Group;
}

const IShaderGroupInternal& ShaderInstance::GetGroup() const
{
	return GetGroupInternal();
}

const vk::SpecializationMapEntry* ShaderGroup::GetMapEntries(uint32_t & count) const
{
	Util::SafeConvert(m_VkEntries.size(), count);
	return m_VkEntries.data();
}
UniformBufferIndex ShaderGroup::FindUniformBuffer(const std::string_view& name) const
{
	const auto& buffers = GetVulkanShader().GetReflectionData().m_UniformBuffers;
	for (auto& buf : buffers)
	{
		if (buf.m_Name != name)
			continue;

		return Util::SafeConvert<UniformBufferIndex>(buf.m_Binding);
	}

	return UniformBufferIndex::Invalid;
}

size_t ShaderGroup::Hasher::operator()(const std::byte * data) const
{
	// Probably more efficient to (ab)use a string_view?
	return Util::hash_value(std::string_view((const char*)data, m_Size));
}

ShaderGroup::ShaderGroup(ShaderType type, const IVulkanShader & shader, const ISpecConstLayout & layout) :
	m_Type(type),
	m_VulkanShader(&shader),
	m_SpecConstLayout(&layout),
	m_Instances(0, Hasher(layout.GetBufferSize()), KeyEqual(layout.GetBufferSize()))
{
	// Actually look up the spec const layout string names here
	const auto& reflData = shader.GetReflectionData();
	size_t entryCount;
	const auto* scEntries = layout.GetEntries(entryCount);
	for (size_t i = 0; i < entryCount; i++)
	{
		const auto& scEntry = scEntries[i];

		const auto found = std::find_if(reflData.m_SpecConstants.begin(), reflData.m_SpecConstants.end(),
			[&](const auto & sc) { return sc.m_Name == scEntry.m_Name; });
		if (found == reflData.m_SpecConstants.end())
		{
			Warning(TF2VULKAN_PREFIX "Unable to find specialization constant %.*s in %s\n",
				PRINTF_SV(scEntry.m_Name), shader.GetName().String());
			continue;
		}

		auto & vkEntry = m_VkEntries.emplace_back();
		vkEntry.constantID = found->m_ConstantID;
		vkEntry.offset = scEntry.m_Offset;
		vkEntry.size = 4;
	}
}

IShaderInstance& ShaderGroup::FindOrCreateInstance(const void* specConstBuf,
	size_t specConstBufSize)
{
	LOG_FUNC();

	auto test = static_cast<const SpecConstLayout*>(m_SpecConstLayout);
	if (specConstBufSize != m_SpecConstLayout->GetBufferSize())
		throw VulkanException("Mismatching specialization constant buffer size", EXCEPTION_DATA());

	const std::byte * byteBuf = reinterpret_cast<const std::byte*>(specConstBuf);

	if (auto found = m_Instances.find(byteBuf); found != m_Instances.end())
		return found->second;

	// Couldn't find a matching instance, create a new one now
	ShaderInstance tmpInst(*this, specConstBuf);
	auto& retVal = m_Instances.emplace(reinterpret_cast<const std::byte*>(tmpInst.GetSpecConstBuffer()), std::move(tmpInst)).first->second;
	retVal.DisallowDeletion();
	return retVal;
}

IShaderGroup & ShaderNextFactory::FindOrCreateShaderGroup(ShaderType type,
	const char* name, const ISpecConstLayout* layout)
{
	LOG_FUNC();

	auto& scLayout = layout ? *layout : *m_EmptySCLayout;

	const ShaderGroupKey key{ name, &scLayout };
	auto& groupMap = m_ShaderInstanceGroups.at(size_t(type));
	if (auto found = groupMap.find(key); found != groupMap.end())
	{
		auto& foundGroup = found->second;
		assert(&foundGroup.GetSpecConstLayout() == &scLayout);
		return found->second;
	}

	// Not found, create now
	return groupMap.emplace(key, ShaderGroup(
		type, g_ShaderManager.FindOrCreateShader(name), scLayout)).first->second;
}

const ISpecConstLayout& ShaderNextFactory::FindOrCreateSpecConstLayout(
	const SpecConstLayoutCreateInfo& ci)
{
	LOG_FUNC();

	return *m_SpecConstLayouts.emplace(SpecConstLayoutCreateInfo{ ci.m_Entries, ci.m_EntryCount }).first;
}
