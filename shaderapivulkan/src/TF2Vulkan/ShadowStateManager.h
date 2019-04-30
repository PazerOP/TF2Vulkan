#pragma once

#include "LogicalState.h"
#include "VertexFormat.h"

#include <TF2Vulkan/Util/Templates.h>

#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishadershadow.h>

#include <unordered_map>

namespace TF2Vulkan
{
	class IShaderShadowInternal : public IShaderShadow
	{
	public:
	};

	class ShadowStateManager : public IShaderShadowInternal
	{
	public:
		void ApplyState(LogicalShadowStateID id, const vk::CommandBuffer& buf);
		void ApplyCurrentState(const vk::CommandBuffer& buf);
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

		LogicalShadowStateID TakeSnapshot();
		bool IsTranslucent(StateSnapshot_t id) const;
		bool IsAlphaTested(StateSnapshot_t id) const;
		bool UsesVertexAndPixelShaders(StateSnapshot_t id) const;
		bool IsDepthWriteEnabled(StateSnapshot_t id) const;

		void SetRenderTargetEx(int rtID, ShaderAPITextureHandle_t colTex, ShaderAPITextureHandle_t depthTex);

		const LogicalShadowState& GetState(StateSnapshot_t id) const;
		const LogicalShadowState& GetState(LogicalShadowStateID id) const;

	protected:
		bool HasStateChanged() const;

	private:
		bool m_Dirty = true;
		LogicalShadowState m_State;

		std::unordered_map<LogicalShadowState, LogicalShadowStateID> m_StatesToIDs;
		std::vector<const LogicalShadowState*> m_IDsToStates;
	};

	extern ShadowStateManager& g_ShadowStateManager;
}
