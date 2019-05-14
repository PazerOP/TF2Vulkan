#include "interface/internal/IStateManagerStatic.h"
#include "interface/internal/IShaderInstanceInternal.h"
#include "IStateManagerDynamic.h"
#include "IStateManagerVulkan.h"
#include "shaders/VulkanShaderManager.h"

#include <TF2Vulkan/Util/Buffer.h>
#include <TF2Vulkan/Util/DirtyVar.h>
#include <TF2Vulkan/Util/interface.h>

#include <unordered_map>
#include <unordered_set>

using namespace TF2Vulkan;
using namespace Util;

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

	class ShadowStateManager final : public IStateManagerStatic
	{
	public:
		ShadowStateManager();

		void ApplyState(LogicalShadowStateID id, IVulkanCommandBuffer& buf);
		void ApplyCurrentState(IVulkanCommandBuffer& buf);
		void SetDefaultState() override final;

		void DepthFunc(ShaderDepthFunc_t func) override final;
		void EnableDepthWrites(bool enable) override final;
		void EnableDepthTest(bool enable) override final;
		void EnablePolyOffset(PolygonOffsetMode_t mode);

		void EnableStencil(bool enable) override final;
		void StencilFunc(ShaderStencilFunc_t func) override final;
		void StencilPassOp(ShaderStencilOp_t op) override final;
		void StencilFailOp(ShaderStencilOp_t op) override final;
		void StencilDepthFailOp(ShaderStencilOp_t op) override final;
		void StencilReference(int reference) override final;
		void StencilMask(int mask) override final;
		void StencilWriteMask(int mask) override final;

		void EnableColorWrites(bool enable) override final;
		void EnableAlphaWrites(bool enable) override final;

		void EnableBlending(bool enable) override final;
		void BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) override final;

		void EnableAlphaTest(bool enable) override final;
		void AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef) override final;

		void PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t mode) override final;

		void EnableCulling(bool enable) override final;

		void EnableConstantColor(bool enable) override final;

		void VertexShaderVertexFormat(uint flags, int texCoordCount,
			int* texCoorDimensions, int userDataSize) override final;

		void SetShaderGroup(ShaderType type, IShaderGroup* instance) override final;

		void EnableLighting(bool enable) override final;

		void EnableSpecular(bool enable) override final;

		void EnableSRGBWrite(bool enable) override final;

		void EnableSRGBRead(Sampler_t sampler, bool enable) override final;

		void EnableVertexBlend(bool enable) override final;

		void OverbrightValue(TextureStage_t stage, float value) override final;
		void EnableTexture(Sampler_t sampler, bool enable) override final;
		void EnableTexGen(TextureStage_t stage, bool enable) override final;
		void TexGen(TextureStage_t stage, ShaderTexGenParam_t param) override final;

		void EnableCustomPixelPipe(bool enable) override final;
		void CustomTextureStages(int stageCount) override final;
		void CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel,
			ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2) override final;

		void DrawFlags(uint drawFlags) override final;

		void EnableAlphaPipe(bool enable) override final;
		void EnableConstantAlpha(bool enable) override final;
		void EnableVertexAlpha(bool enable) override final;
		void EnableTextureAlpha(TextureStage_t stage, bool enable) override final;

		void EnableBlendingSeparateAlpha(bool enable) override final;
		void BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) override final;
		void FogMode(ShaderFogMode_t fogMode) override final;

		void SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource) override final;

		void SetMorphFormat(MorphFormat_t format) override final;

		void DisableFogGammaCorrection(bool disable) override final;

		void EnableAlphaToCoverage(bool enable) override final;

		void SetShadowDepthFiltering(Sampler_t stage) override final;

		void BlendOp(ShaderBlendOp_t op) override final;
		void BlendOpSeparateAlpha(ShaderBlendOp_t op) override final;

		LogicalShadowStateID TakeSnapshot() override;
		bool IsTranslucent(LogicalShadowStateID id) const override;
		bool IsAlphaTested(LogicalShadowStateID id) const override;
		bool UsesVertexAndPixelShaders(LogicalShadowStateID id) const override;
		bool IsDepthWriteEnabled(LogicalShadowStateID id) const override;

		bool IsAnyRenderTargetBound() const override;
		void SetRenderTargetEx(int rtID, ShaderAPITextureHandle_t colTex, ShaderAPITextureHandle_t depthTex) override;

		void SetState(LogicalShadowStateID id) override;
		using IStateManagerStatic::GetState;
		const LogicalShadowState& GetState(LogicalShadowStateID id) const override;
		const LogicalShadowState& GetCurrentState() const override { return m_State; }

		const TF2Vulkan::IVulkanShader& GetPixelShader() const override;
		const TF2Vulkan::IVulkanShader& GetVertexShader() const override;

		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const ISpecConstLayout* layout) override;
		const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutEntry* entries, size_t count) override;

	protected:
		bool HasStateChanged() const;

	private:
		bool m_Dirty = true;
		LogicalShadowState m_State;

		std::array<std::unordered_map<CUtlSymbolDbg, ShaderGroup>, 2> m_ShaderInstanceGroups;
		const SpecConstLayout* m_EmptySCLayout;
		std::unordered_set<SpecConstLayout> m_SpecConstLayouts;

		std::unordered_map<LogicalShadowState, LogicalShadowStateID> m_StatesToIDs;
		std::vector<const LogicalShadowState*> m_IDsToStates;
	};
}

static ShadowStateManager s_SSM;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShadowStateManager, IShaderShadow, SHADERSHADOW_INTERFACE_VERSION, s_SSM);

IStateManagerStatic& TF2Vulkan::g_StateManagerStatic = s_SSM;

ShadowStateManager::ShadowStateManager()
{
	m_EmptySCLayout = &*m_SpecConstLayouts.emplace(SpecConstLayout(nullptr, 0)).first;
}

void ShadowStateManager::ApplyState(LogicalShadowStateID id, IVulkanCommandBuffer& buf)
{
	LOG_FUNC();
	g_StateManagerVulkan.ApplyState(GetState(id), g_StateManagerDynamic.GetDynamicState(), buf);
}

void ShadowStateManager::ApplyCurrentState(IVulkanCommandBuffer& buf)
{
	LOG_FUNC();
	ApplyState(TakeSnapshot(), buf);
}

void ShadowStateManager::SetDefaultState()
{
	LOG_FUNC();

	m_State = {};
	m_Dirty = true;
}

void ShadowStateManager::DepthFunc(ShaderDepthFunc_t func)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_DepthCompareFunc, func, m_Dirty);
}

void ShadowStateManager::EnableDepthWrites(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_DepthWrite, enable, m_Dirty);
}

void ShadowStateManager::EnableDepthTest(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_DepthTest, enable, m_Dirty);
}

void ShadowStateManager::EnablePolyOffset(PolygonOffsetMode_t mode)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_RSPolyOffsetMode, mode, m_Dirty);
}

void ShadowStateManager::EnableStencil(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilFunc(ShaderStencilFunc_t func)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilPassOp(ShaderStencilOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilFailOp(ShaderStencilOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilDepthFailOp(ShaderStencilOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilReference(int reference)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilMask(int mask)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::StencilWriteMask(int mask)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableColorWrites(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMColorWrite, enable, m_Dirty);
}

void ShadowStateManager::EnableAlphaWrites(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMAlphaWrite, enable, m_Dirty);
}

void ShadowStateManager::EnableBlending(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMAlphaBlending, enable, m_Dirty);
}

void ShadowStateManager::BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMSrcFactor, srcFactor, m_Dirty);
	SetDirtyVar(m_State.m_OMDstFactor, dstFactor, m_Dirty);
}

void ShadowStateManager::EnableAlphaTest(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMAlphaTest, enable, m_Dirty);
}

void ShadowStateManager::AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMAlphaTestFunc, alphaFunc, m_Dirty);
	SetDirtyVar(m_State.m_OMAlphaTestRef, int(alphaRef * 255), m_Dirty);
}

void ShadowStateManager::PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t mode)
{
	LOG_FUNC();
	if (face == SHADER_POLYMODEFACE_FRONT || face == SHADER_POLYMODEFACE_FRONT_AND_BACK)
		SetDirtyVar(m_State.m_RSFrontFacePolyMode, mode, m_Dirty);
	if (face == SHADER_POLYMODEFACE_BACK || face == SHADER_POLYMODEFACE_FRONT_AND_BACK)
		SetDirtyVar(m_State.m_RSBackFacePolyMode, mode, m_Dirty);
}

void ShadowStateManager::EnableCulling(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_RSBackFaceCulling, enable, m_Dirty);
}

void ShadowStateManager::EnableConstantColor(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::VertexShaderVertexFormat(uint flags,
	int texCoordCount, int* texCoordDimensions, int userDataSize)
{
	LOG_FUNC();

	VertexFormat fmt(flags | VERTEX_USERDATA_SIZE(userDataSize));

	for (int i = 0; i < texCoordCount; i++)
	{
		auto dim = texCoordDimensions ? texCoordDimensions[i] : 2;
		fmt.m_BaseFmt |= VERTEX_TEXCOORD_SIZE(i, dim);
	}

	auto& oldFmt = m_State.m_VSVertexFormat;
	assert(oldFmt == VERTEX_FORMAT_UNKNOWN || oldFmt == fmt); // TODO: Are we supposed to merge flags/texcoord dimensions?
	SetDirtyVar(oldFmt, fmt, m_Dirty);
}

void ShadowStateManager::EnableLighting(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableSpecular(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableSRGBWrite(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_OMSRGBWrite, enable, m_Dirty);
}

void ShadowStateManager::EnableSRGBRead(Sampler_t sampler, bool enable)
{
	LOG_FUNC();
	ENSURE(sampler >= Sampler_t(0) && sampler < Sampler_t(std::size(m_State.m_PSSamplers)));
	auto& s = m_State.m_PSSamplers[sampler];
	if (s.m_SRGBRead != enable)
	{
		s.m_SRGBRead = enable;
		m_Dirty = true;
	}
}

void ShadowStateManager::EnableVertexBlend(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::OverbrightValue(TextureStage_t stage, float value)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableTexture(Sampler_t sampler, bool enable)
{
	LOG_FUNC();
	ENSURE(sampler >= Sampler_t(0) && sampler < Sampler_t(std::size(m_State.m_PSSamplers)));
	auto & s = m_State.m_PSSamplers[sampler];
	if (s.m_Enabled != enable)
	{
		s.m_Enabled = enable;
		m_Dirty = true;
	}
}

void ShadowStateManager::EnableTexGen(TextureStage_t stage, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::TexGen(TextureStage_t stage, ShaderTexGenParam_t param)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableCustomPixelPipe(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::CustomTextureStages(int stageCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel, ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::DrawFlags(uint drawFlags)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableAlphaPipe(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableConstantAlpha(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableVertexAlpha(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableTextureAlpha(TextureStage_t stage, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableBlendingSeparateAlpha(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::FogMode(ShaderFogMode_t fogMode)
{
	LOG_FUNC();
	SetDirtyVar(m_State.m_FogMode, fogMode, m_Dirty);
}

void ShadowStateManager::SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::SetMorphFormat(MorphFormat_t format)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::DisableFogGammaCorrection(bool disable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::EnableAlphaToCoverage(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::SetShadowDepthFiltering(Sampler_t stage)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::BlendOp(ShaderBlendOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShadowStateManager::BlendOpSeparateAlpha(ShaderBlendOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

LogicalShadowStateID ShadowStateManager::TakeSnapshot()
{
	if (auto found = m_StatesToIDs.find(m_State); found != m_StatesToIDs.end())
		return found->second;

	// Couldn't find it, add it now
	LogicalShadowStateID nextID = LogicalShadowStateID(m_IDsToStates.size());
	auto emplaced = m_StatesToIDs.emplace(m_State, nextID);
	m_IDsToStates.push_back(&emplaced.first->first);

	return nextID;
}

bool ShadowStateManager::IsTranslucent(LogicalShadowStateID id) const
{
	// TODO: How is "is translucent" actually computed?
	return GetState(id).m_OMAlphaBlending;
}

bool ShadowStateManager::IsAlphaTested(LogicalShadowStateID id) const
{
	return GetState(id).m_OMAlphaTest;
}

bool ShadowStateManager::UsesVertexAndPixelShaders(LogicalShadowStateID id) const
{
	const auto& state = GetState(id);

	assert(!state.m_VSShader == !state.m_PSShader);
	return !!state.m_VSShader || !!state.m_PSShader;
}

bool ShadowStateManager::IsDepthWriteEnabled(LogicalShadowStateID id) const
{
	return GetState(id).m_DepthWrite;
}

void ShadowStateManager::SetRenderTargetEx(int rtID, ShaderAPITextureHandle_t colTex, ShaderAPITextureHandle_t depthTex)
{
	Util::SetDirtyVar(m_State.m_OMColorRTs, rtID, colTex, m_Dirty);
	Util::SetDirtyVar(m_State.m_OMDepthRT, depthTex, m_Dirty);
}

bool ShadowStateManager::HasStateChanged() const
{
	return m_Dirty;
}

void ShadowStateManager::SetState(LogicalShadowStateID id)
{
	LOG_FUNC();
	m_State = *m_IDsToStates.at(Util::SafeConvert<size_t>(id));
}

auto ShadowStateManager::GetState(LogicalShadowStateID id) const -> const LogicalShadowState &
{
	return *m_IDsToStates.at(Util::SafeConvert<size_t>(id));
}

bool ShadowStateManager::IsAnyRenderTargetBound() const
{
	for (auto& id : m_State.m_OMColorRTs)
	{
		if (id >= 0)
			return true;
	}

	return false;
}

const IVulkanShader& ShadowStateManager::GetPixelShader() const
{
	return m_State.m_PSShader->GetVulkanShader();
}
const IVulkanShader& ShadowStateManager::GetVertexShader() const
{
	return m_State.m_VSShader->GetVulkanShader();
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

ShaderInstance::ShaderInstance(const ShaderGroup& group, const void* specConstBuffer) :
	m_Group(&group)
{
	const auto bufSize = group.GetSpecConstLayout().GetBufferSize();
	auto buf = std::make_unique<std::byte[]>(bufSize);
	memcpy(buf.get(), specConstBuffer, bufSize);
}

void ShaderInstance::GetSpecializationInfo(vk::SpecializationInfo& info) const
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

const vk::SpecializationMapEntry* ShaderGroup::GetMapEntries(uint32_t& count) const
{
	Util::SafeConvert(m_VkEntries.size(), count);
	return m_VkEntries.data();
}

size_t ShaderGroup::Hasher::operator()(const std::byte* data) const
{
	// Probably more efficient to (ab)use a string_view?
	__debugbreak(); // TODO: VERIFY THIS WORKS
	return Util::hash_value(std::string_view((const char*)data, m_Size));
}

ShaderGroup::ShaderGroup(ShaderType type, const IVulkanShader& shader, const SpecConstLayout& layout) :
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

		auto& vkEntry = m_VkEntries.emplace_back();
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

	const std::byte* byteBuf = reinterpret_cast<const std::byte*>(specConstBuf);

	if (auto found = m_Instances.find(byteBuf); found != m_Instances.end())
		return found->second;

	// Couldn't find a matching instance, create a new one now
	return m_Instances.emplace(byteBuf, ShaderInstance(*this, specConstBuf)).first->second;
}

IShaderGroup& ShadowStateManager::FindOrCreateShaderGroup(ShaderType type,
	const char* name, const ISpecConstLayout* layout)
{
	auto& groupMap = m_ShaderInstanceGroups.at(size_t(type));
	if (auto found = groupMap.find(name); found != groupMap.end())
	{
		auto& foundGroup = found->second;
		assert(&foundGroup.GetSpecConstLayout() == layout);
		return found->second;
	}

	auto& scLayout = layout ? *assert_cast<const SpecConstLayout*>(layout) : *m_EmptySCLayout;

	// Not found, create now
	return groupMap.emplace(name, ShaderGroup(
		type, g_ShaderManager.FindOrCreateShader(name), scLayout)).first->second;
}

const ISpecConstLayout& ShadowStateManager::FindOrCreateSpecConstLayout(
	const SpecConstLayoutEntry* entries, size_t count)
{
	return *m_SpecConstLayouts.emplace(SpecConstLayout(entries, count)).first;
}

bool SpecConstLayout::operator==(const SpecConstLayout& other) const
{
	if (m_EntryCount != other.m_EntryCount)
		return false;

	auto result = std::equal(begin(), end(), other.begin());
	assert(!result || (m_BufferSize == other.m_BufferSize));
	return result;
}

void ShadowStateManager::SetShaderGroup(ShaderType type, IShaderGroup* instance)
{
	LOG_FUNC();

	switch (type)
	{
	case ShaderType::Vertex:
		Util::SetDirtyVar(m_State.m_VSShader, assert_cast<IShaderGroupInternal*>(instance), m_Dirty);
		break;
	case ShaderType::Pixel:
		Util::SetDirtyVar(m_State.m_PSShader, assert_cast<IShaderGroupInternal*>(instance), m_Dirty);
		break;

	default:
		assert(!"Unknown shader type");
	}
}
