#include "ShadowStateManager.h"
#include <TF2Vulkan/Util/DirtyVar.h>

using namespace TF2Vulkan;
using namespace Util;

void ShadowStateManager::ApplyState(ShadowStateID id, const vk::CommandBuffer& buf)
{
	LOG_FUNC();

	if (m_IDsToPipelines.size() <= size_t(id))
		m_IDsToPipelines.resize(size_t(id) + 1);

	auto& pl = m_IDsToPipelines[size_t(id)];
	if (!pl)
		pl = CreatePipeline(*m_IDsToStates[size_t(id)]);

	buf.bindPipeline(vk::PipelineBindPoint::eGraphics, pl.get());
}

void ShadowStateManager::ApplyCurrentState(const vk::CommandBuffer& buf)
{
	LOG_FUNC();
	ApplyState(TakeSnapshot(), buf);
}

void ShadowStateManager::SetDefaultState()
{
	LOG_FUNC();

	m_Settings = {};

	m_PipelineDirty = true;
	m_FogDirty = true;
}

void ShadowStateManager::DepthFunc(ShaderDepthFunc_t func)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_DepthStencil.m_Depth.m_CompareFunc, func, m_PipelineDirty);
}

void ShadowStateManager::EnableDepthWrites(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_DepthStencil.m_Depth.m_DepthWrite, enable, m_PipelineDirty);
}

void ShadowStateManager::EnableDepthTest(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_DepthStencil.m_Depth.m_DepthTest, enable, m_PipelineDirty);
}

void ShadowStateManager::EnablePolyOffset(PolygonOffsetMode_t mode)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Rasterizer.m_OffsetMode, mode, m_PipelineDirty);
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
	SetDirtyVar(m_Settings.m_Blend.m_ColorWrite, enable, m_PipelineDirty);
}

void ShadowStateManager::EnableAlphaWrites(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Blend.m_AlphaWrite, enable, m_PipelineDirty);
}

void ShadowStateManager::EnableBlending(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Blend.m_AlphaBlending, enable, m_PipelineDirty);
}

void ShadowStateManager::BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Blend.m_SrcFactor, srcFactor, m_PipelineDirty);
	SetDirtyVar(m_Settings.m_Blend.m_DstFactor, dstFactor, m_PipelineDirty);
}

void ShadowStateManager::EnableAlphaTest(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Blend.m_AlphaTest, enable, m_PipelineDirty);
}

void ShadowStateManager::AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Blend.m_AlphaTestFunc, alphaFunc, m_PipelineDirty);
	SetDirtyVar(m_Settings.m_Blend.m_AlphaTestRef, int(alphaRef * 255), m_PipelineDirty);
}

void ShadowStateManager::PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t mode)
{
	LOG_FUNC();
	if (face == SHADER_POLYMODEFACE_FRONT || face == SHADER_POLYMODEFACE_FRONT_AND_BACK)
		SetDirtyVar(m_Settings.m_Rasterizer.m_FrontFace.m_PolyMode, mode, m_PipelineDirty);
	if (face == SHADER_POLYMODEFACE_BACK || face == SHADER_POLYMODEFACE_FRONT_AND_BACK)
		SetDirtyVar(m_Settings.m_Rasterizer.m_BackFace.m_PolyMode, mode, m_PipelineDirty);
}

void ShadowStateManager::EnableCulling(bool enable)
{
	LOG_FUNC();
	SetDirtyVar(m_Settings.m_Rasterizer.m_BackfaceCulling, enable, m_PipelineDirty);
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

	auto& oldFmt = m_Settings.m_VertexShader.m_VertexFormat;
	assert(oldFmt == VERTEX_FORMAT_UNKNOWN || oldFmt == fmt); // TODO: Are we supposed to merge flags/texcoord dimensions?
	SetDirtyVar(oldFmt, fmt, m_PipelineDirty);
}

void ShadowStateManager::SetVertexShader(const char* filename, int staticIndex)
{
	LOG_FUNC();

	SetDirtyVar(m_Settings.m_VertexShader.m_Name, filename, m_PipelineDirty);
	SetDirtyVar(m_Settings.m_VertexShader.m_StaticIndex, staticIndex, m_PipelineDirty);
}

void ShadowStateManager::SetPixelShader(const char* filename, int staticIndex)
{
	LOG_FUNC();

	SetDirtyVar(m_Settings.m_PixelShader.m_Name, filename, m_PipelineDirty);
	SetDirtyVar(m_Settings.m_PixelShader.m_StaticIndex, staticIndex, m_PipelineDirty);
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
	SetDirtyVar(m_Settings.m_Blend.m_SRGBWrite, enable, m_PipelineDirty);
}

void ShadowStateManager::EnableSRGBRead(Sampler_t sampler, bool enable)
{
	LOG_FUNC();
	auto& s = m_Settings.m_PixelShader.m_Samplers.at(sampler);
	if (s.m_SRGBRead != enable)
	{
		s.m_SRGBRead = enable;
		m_PipelineDirty = true;
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
	auto& s = m_Settings.m_PixelShader.m_Samplers.at(sampler);
	if (s.m_Enabled != enable)
	{
		s.m_Enabled = enable;
		m_PipelineDirty = true;
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
	SetDirtyVar(m_Settings.m_Fog.m_Mode, fogMode, m_FogDirty);
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

ShadowStateID ShadowStateManager::TakeSnapshot()
{
	if (auto found = m_StatesToIDs.find(m_Settings); found != m_StatesToIDs.end())
		return found->second;

	// Couldn't find it, add it now
	ShadowStateID nextID = ShadowStateID(m_IDsToStates.size());
	auto emplaced = m_StatesToIDs.emplace(m_Settings, nextID);
	m_IDsToStates.push_back(&emplaced.first->first);

	return nextID;
}

bool ShadowStateManager::IsTranslucent(StateSnapshot_t id) const
{
	// TODO: How is "is translucent" actually computed?
	return GetState(id).m_Blend.m_AlphaBlending;
}

bool ShadowStateManager::IsAlphaTested(StateSnapshot_t id) const
{
	return GetState(id).m_Blend.m_AlphaTest;
}

bool ShadowStateManager::UsesVertexAndPixelShaders(StateSnapshot_t id) const
{
	const auto& state = GetState(id);

	assert(!state.m_VertexShader.m_Name == !state.m_PixelShader.m_Name);
	return !!state.m_VertexShader.m_Name;
}

bool ShadowStateManager::IsDepthWriteEnabled(StateSnapshot_t id) const
{
	return GetState(id).m_DepthStencil.m_Depth.m_DepthWrite;
}

void ShadowStateManager::SetRenderTargetEx(int rtID, ShaderAPITextureHandle_t colTex, ShaderAPITextureHandle_t depthTex)
{
	Util::SetDirtyVar(m_Settings.m_RenderPass.m_RenderTargetColors, rtID, colTex, m_PipelineDirty);
	Util::SetDirtyVar(m_Settings.m_RenderPass.m_RenderTargetDepth, depthTex, m_PipelineDirty);
}

bool ShadowStateManager::HasStateChanged() const
{
	return m_PipelineDirty;
}

auto ShadowStateManager::GetState(StateSnapshot_t id) const -> const ShadowState &
{
	return *m_IDsToStates.at(Util::SafeConvert<size_t>(id));
}

size_t ShadowStateManager::ShadowStateHasher::operator()(const FogParams& s) const
{
	return Util::hash_multi(
		s.m_Mode
	);
}

size_t ShadowStateManager::ShadowStateHasher::operator()(const ShadowState& s) const
{
	return Util::hash_combine(
		{
			Util::hash_value(static_cast<const PipelineSettings&>(s)),
			operator()(s.m_Fog)
		});
}
