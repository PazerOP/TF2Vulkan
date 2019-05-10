#include "interface/internal/IStateManagerStatic.h"
#include "IStateManagerDynamic.h"
#include "IStateManagerVulkan.h"
#include "shaders/VulkanShaderManager.h"

#include <TF2Vulkan/Util/DirtyVar.h>
#include <TF2Vulkan/Util/interface.h>

#include <unordered_map>

using namespace TF2Vulkan;
using namespace Util;

namespace
{
	class ShadowStateManager final : public IStateManagerStatic
	{
	public:
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

		void SetVertexShader(const char* filename, int staticIndex) override final;
		void SetPixelShader(const char* filename, int staticIndex) override final;

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

		const TF2Vulkan::IVulkanShader& GetPixelShader() const override;
		const TF2Vulkan::IVulkanShader& GetVertexShader() const override;

	protected:
		bool HasStateChanged() const;

	private:
		bool m_Dirty = true;
		LogicalShadowState m_State;

		std::unordered_map<LogicalShadowState, LogicalShadowStateID> m_StatesToIDs;
		std::vector<const LogicalShadowState*> m_IDsToStates;
	};
}

static ShadowStateManager s_SSM;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShadowStateManager, IShaderShadow, SHADERSHADOW_INTERFACE_VERSION, s_SSM);

IStateManagerStatic& TF2Vulkan::g_StateManagerStatic = s_SSM;

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

void ShadowStateManager::SetVertexShader(const char* filename, int staticIndex)
{
	LOG_FUNC();

	SetDirtyVar(m_State.m_VSName, filename, m_Dirty);
	SetDirtyVar(m_State.m_VSStaticIndex, staticIndex, m_Dirty);
}

void ShadowStateManager::SetPixelShader(const char* filename, int staticIndex)
{
	LOG_FUNC();

	SetDirtyVar(m_State.m_PSName, filename, m_Dirty);
	SetDirtyVar(m_State.m_PSStaticIndex, staticIndex, m_Dirty);
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

	assert(!state.m_VSName == !state.m_PSName);
	return !!state.m_VSName;
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

const TF2Vulkan::IVulkanShader& ShadowStateManager::GetPixelShader() const
{
	return g_ShaderManager.FindOrCreateShader(m_State.m_PSName);
}
const TF2Vulkan::IVulkanShader& ShadowStateManager::GetVertexShader() const
{
	return g_ShaderManager.FindOrCreateShader(m_State.m_VSName);
}
