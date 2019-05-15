#include "interface/internal/IShaderInstanceInternal.h"
#include "shaders/VulkanShaderManager.h"

#include <TF2Vulkan/IShaderNextInstanceMgr.h>
#include <TF2Vulkan/ISpecConstLayout.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <unordered_map>
#include <unordered_set>

using namespace TF2Vulkan;

namespace
{
	struct SpecConstLayout final : ISpecConstLayout
	{
		SpecConstLayout(const SpecConstLayoutEntry* entries, size_t entryCount);

		bool operator==(const SpecConstLayout& other) const;

		// Inherited via ISpecConstLayout
		const SpecConstLayoutEntry* GetEntries(size_t& count) const override { count = m_EntryCount; return m_Entries; }
		size_t GetEntryCount() const { return m_EntryCount; }
		size_t GetBufferSize() const override { return m_BufferSize; }

		auto begin() const { return m_Entries; }
		auto end() const { return m_Entries + m_EntryCount; }

	private:
		const SpecConstLayoutEntry* m_Entries;
		size_t m_EntryCount;
		size_t m_BufferSize;
	};
}

template<>
struct ::std::hash<SpecConstLayout>
{
	size_t operator()(const SpecConstLayout& layout) const
	{
		return Util::hash_range(std::begin(layout), std::end(layout));
	}
};

namespace
{
	struct ShaderGroup;
	struct ShaderInstance final : IShaderInstanceInternal
	{
		ShaderInstance(const ShaderGroup& group, const void* specConstBuffer);

		// Inherited via IShaderInstanceInternal
		const ShaderGroup& GetGroupInternal() const;
		const IShaderGroupInternal& GetGroup() const override;
		const void* GetSpecConstBuffer() const override { return m_SpecConstBuffer.get(); }
		void GetSpecializationInfo(vk::SpecializationInfo& info) const override;

	private:
		const ShaderGroup* m_Group = nullptr;
		std::unique_ptr<const std::byte[]> m_SpecConstBuffer;
	};

	struct ShaderGroup final : IShaderGroupInternal
	{
		ShaderGroup(ShaderType type, const IVulkanShader& shader, const SpecConstLayout& layout);

		// Inherited via IShaderGroupInternal
		const char* GetName() const override { return GetVulkanShader().GetName().String(); }
		const SpecConstLayout& GetSpecConstLayout() const override { return *m_SpecConstLayout; }
		ShaderType GetShaderType() const override { return m_Type; }
		IShaderInstance& FindOrCreateInstance(const void* specConstBuf, size_t specConstBufSize) override;
		const IVulkanShader& GetVulkanShader() const override { return *m_VulkanShader; }
		const vk::SpecializationMapEntry* GetMapEntries(uint32_t& count) const;

	private:
		ShaderType m_Type;
		const IVulkanShader* m_VulkanShader = nullptr;
		const SpecConstLayout* m_SpecConstLayout = nullptr;
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

	class ShaderNextInstanceMgr final : IShaderNextInstanceMgr
	{
	public:
		ShaderNextInstanceMgr();

		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const ISpecConstLayout* layout) override;
		const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutEntry* entries, size_t count) override;

	private:
		std::array<std::unordered_map<CUtlSymbolDbg, ShaderGroup>, 2> m_ShaderInstanceGroups;
		const SpecConstLayout* m_EmptySCLayout;
		std::unordered_set<SpecConstLayout> m_SpecConstLayouts;
	};
}

ShaderNextInstanceMgr::ShaderNextInstanceMgr()
{
	m_EmptySCLayout = &*m_SpecConstLayouts.emplace(SpecConstLayout(nullptr, 0)).first;
}

SpecConstLayout::SpecConstLayout(const SpecConstLayoutEntry* entries, size_t entryCount) :
	m_Entries(entries),
	m_EntryCount(entryCount)
{
	assert(!!m_Entries == !!m_EntryCount);

	// Make sure there's no padding and they're in order
	size_t nextOffset = 0;
	for (size_t i = 0; i < entryCount; i++)
	{
		assert(entries[i].m_Offset == nextOffset);
		nextOffset += 4;
	}

	m_BufferSize = nextOffset;
}

ShaderInstance::ShaderInstance(const ShaderGroup & group, const void* specConstBuffer) :
	m_Group(&group)
{
	const auto bufSize = group.GetSpecConstLayout().GetBufferSize();
	auto buf = std::make_unique<std::byte[]>(bufSize);
	memcpy(buf.get(), specConstBuffer, bufSize);
}

void ShaderInstance::GetSpecializationInfo(vk::SpecializationInfo & info) const
{
	info.dataSize = m_Group->GetSpecConstLayout().GetBufferSize();
	info.pData = m_SpecConstBuffer.get();
	info.pMapEntries = GetGroupInternal().GetMapEntries(info.mapEntryCount);
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

size_t ShaderGroup::Hasher::operator()(const std::byte * data) const
{
	// Probably more efficient to (ab)use a string_view?
	__debugbreak(); // TODO: VERIFY THIS WORKS
	return Util::hash_value(std::string_view((const char*)data, m_Size));
}

ShaderGroup::ShaderGroup(ShaderType type, const IVulkanShader & shader, const SpecConstLayout & layout) :
	m_Type(type),
	m_VulkanShader(&shader),
	m_SpecConstLayout(&layout),
	m_Instances(0, Hasher(layout.GetBufferSize()), KeyEqual(layout.GetBufferSize()))
{
	// Actually look up the spec const layout string names here
	const auto& reflData = shader.GetReflectionData();
	for (const auto& scEntry : layout)
	{
		const auto found = std::find_if(reflData.m_SpecConstants.begin(), reflData.m_SpecConstants.end(),
			[&](const auto & sc) { return sc.m_Name == scEntry.m_Name; });
		if (found == reflData.m_SpecConstants.end())
			continue;

		auto & vkEntry = m_VkEntries.emplace_back();
		vkEntry.constantID = found->m_ConstantID;
		vkEntry.offset = scEntry.m_Offset;
		vkEntry.size = 4;
	}
}

IShaderInstance& ShaderGroup::FindOrCreateInstance(const void* specConstBuf,
	size_t specConstBufSize)
{
	if (specConstBufSize != m_SpecConstLayout->GetBufferSize())
		throw VulkanException("Mismatching specialization constant buffer size", EXCEPTION_DATA());

	const std::byte * byteBuf = reinterpret_cast<const std::byte*>(specConstBuf);

	if (auto found = m_Instances.find(byteBuf); found != m_Instances.end())
		return found->second;

	// Couldn't find a matching instance, create a new one now
	return m_Instances.emplace(byteBuf, ShaderInstance(*this, specConstBuf)).first->second;
}

IShaderGroup & ShaderNextInstanceMgr::FindOrCreateShaderGroup(ShaderType type,
	const char* name, const ISpecConstLayout * layout)
{
	auto& scLayout = layout ? *assert_cast<const SpecConstLayout*>(layout) : *m_EmptySCLayout;

	auto& groupMap = m_ShaderInstanceGroups.at(size_t(type));
	if (auto found = groupMap.find(name); found != groupMap.end())
	{
		auto& foundGroup = found->second;
		assert(&foundGroup.GetSpecConstLayout() == &scLayout);
		return found->second;
	}

	// Not found, create now
	return groupMap.emplace(name, ShaderGroup(
		type, g_ShaderManager.FindOrCreateShader(name), scLayout)).first->second;
}

const ISpecConstLayout& ShaderNextInstanceMgr::FindOrCreateSpecConstLayout(
	const SpecConstLayoutEntry * entries, size_t count)
{
	return *m_SpecConstLayouts.emplace(SpecConstLayout(entries, count)).first;
}

bool SpecConstLayout::operator==(const SpecConstLayout & other) const
{
	if (m_EntryCount != other.m_EntryCount)
		return false;

	auto result = std::equal(begin(), end(), other.begin());
	assert(!result || (m_BufferSize == other.m_BufferSize));
	return result;
}
