#include "ShadowStateManager.h"
#include "IStateManagerDynamic.h"
#include "IStateManagerVulkan.h"
#include <TF2Vulkan/Util/DirtyVar.h>

using namespace TF2Vulkan;
using namespace Util;

void ShadowStateManager::ApplyState(LogicalShadowStateID id, const vk::CommandBuffer& buf)
{
	LOG_FUNC();
	if (m_Dirty)
	{
		g_StateManagerVulkan.ApplyState(
			id, GetState(id), g_StateManagerDynamic.GetDynamicState(), buf);
	}
}

void ShadowStateManager::ApplyCurrentState(const vk::CommandBuffer& buf)
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

	if (texCoordDimensions)
	{
		for (int i = 0; i < texCoordCount; i++)
			fmt.m_BaseFmt |= VERTEX_TEXCOORD_SIZE(i, texCoordDimensions[i]);
	}
	else if (texCoordCount)
	{
		Warning(TF2VULKAN_PREFIX "texCoordDimensions was nullptr, but texCoordCount was %i\n", texCoordCount);
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

bool ShadowStateManager::IsTranslucent(StateSnapshot_t id) const
{
	// TODO: How is "is translucent" actually computed?
	return GetState(id).m_OMAlphaBlending;
}

bool ShadowStateManager::IsAlphaTested(StateSnapshot_t id) const
{
	return GetState(id).m_OMAlphaTest;
}

bool ShadowStateManager::UsesVertexAndPixelShaders(StateSnapshot_t id) const
{
	const auto& state = GetState(id);

	assert(!state.m_VSName == !state.m_PSName);
	return !!state.m_VSName;
}

bool ShadowStateManager::IsDepthWriteEnabled(StateSnapshot_t id) const
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

auto ShadowStateManager::GetState(StateSnapshot_t id) const -> const LogicalShadowState &
{
	return GetState(Util::SafeConvert<LogicalShadowStateID>(id));
}
auto ShadowStateManager::GetState(LogicalShadowStateID id) const -> const LogicalShadowState &
{
	return *m_IDsToStates.at(Util::SafeConvert<size_t>(id));
}
