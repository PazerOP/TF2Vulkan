#include <shaderapi/ishadershadow.h>

#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/Placeholders.h>

namespace
{
	class ShaderShadow final : public IShaderShadow
	{
	public:
		void SetDefaultState() override;

		void DepthFunc(ShaderDepthFunc_t func) override;
		void EnableDepthWrites(bool enable) override;
		void EnableDepthTest(bool enable) override;
		void EnablePolyOffset(PolygonOffsetMode_t mode);

		void EnableStencil(bool enable) override;
		void StencilFunc(ShaderStencilFunc_t func) override;
		void StencilPassOp(ShaderStencilOp_t op) override;
		void StencilFailOp(ShaderStencilOp_t op) override;
		void StencilDepthFailOp(ShaderStencilOp_t op) override;
		void StencilReference(int reference) override;
		void StencilMask(int mask) override;
		void StencilWriteMask(int mask) override;

		void EnableColorWrites(bool enable) override;
		void EnableAlphaWrites(bool enable) override;

		void EnableBlending(bool enable) override;
		void BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) override;

		void EnableAlphaTest(bool enable) override;
		void AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef) override;

		void PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t mode) override;

		void EnableCulling(bool enable) override;

		void EnableConstantColor(bool enable) override;

		void VertexShaderVertexFormat(uint flags, int texCoordCount,
			int* texCoorDimensions, int userDataSize) override;

		void SetVertexShader(const char* filename, int staticIndex) override;
		void SetPixelShader(const char* filename, int staticIndex) override;

		void EnableLighting(bool enable) override;

		void EnableSpecular(bool enable) override;

		void EnableSRGBWrite(bool enable) override;

		void EnableSRGBRead(Sampler_t sampler, bool enable) override;

		void EnableVertexBlend(bool enable) override;

		void OverbrightValue(TextureStage_t stage, float value) override;
		void EnableTexture(Sampler_t sampler, bool enable) override;
		void EnableTexGen(TextureStage_t stage, bool enable) override;
		void TexGen(TextureStage_t stage, ShaderTexGenParam_t param) override;

		void EnableCustomPixelPipe(bool enable) override;
		void CustomTextureStages(int stageCount) override;
		void CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel,
			ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2) override;

		void DrawFlags(uint drawFlags) override;

		void EnableAlphaPipe(bool enable) override;
		void EnableConstantAlpha(bool enable) override;
		void EnableVertexAlpha(bool enable) override;
		void EnableTextureAlpha(TextureStage_t stage, bool enable) override;

		void EnableBlendingSeparateAlpha(bool enable) override;
		void BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor) override;
		void FogMode(ShaderFogMode_t fogMode) override;

		void SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource) override;

		void SetMorphFormat(MorphFormat_t format) override;

		void DisableFogGammaCorrection(bool disable) override;

		void EnableAlphaToCoverage(bool enable) override;

		void SetShadowDepthFiltering(Sampler_t stage) override;

		void BlendOp(ShaderBlendOp_t op) override;
		void BlendOpSeparateAlpha(ShaderBlendOp_t op) override;
	};
}

EXPOSE_SINGLE_INTERFACE(ShaderShadow, IShaderShadow, SHADERSHADOW_INTERFACE_VERSION);

void ShaderShadow::SetDefaultState()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::DepthFunc(ShaderDepthFunc_t func)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableDepthWrites(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableDepthTest(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnablePolyOffset(PolygonOffsetMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableStencil(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilFunc(ShaderStencilFunc_t func)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilPassOp(ShaderStencilOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilFailOp(ShaderStencilOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilDepthFailOp(ShaderStencilOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilReference(int reference)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilMask(int mask)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::StencilWriteMask(int mask)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableColorWrites(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableAlphaWrites(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableBlending(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableAlphaTest(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableCulling(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableConstantColor(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::VertexShaderVertexFormat(uint flags, int texCoordCount, int* texCoorDimensions, int userDataSize)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::SetVertexShader(const char* filename, int staticIndex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::SetPixelShader(const char* filename, int staticIndex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableLighting(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableSpecular(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableSRGBWrite(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableSRGBRead(Sampler_t sampler, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableVertexBlend(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::OverbrightValue(TextureStage_t stage, float value)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableTexture(Sampler_t sampler, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableTexGen(TextureStage_t stage, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::TexGen(TextureStage_t stage, ShaderTexGenParam_t param)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableCustomPixelPipe(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::CustomTextureStages(int stageCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel, ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::DrawFlags(uint drawFlags)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableAlphaPipe(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableConstantAlpha(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableVertexAlpha(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableTextureAlpha(TextureStage_t stage, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableBlendingSeparateAlpha(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::FogMode(ShaderFogMode_t fogMode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::SetMorphFormat(MorphFormat_t format)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::DisableFogGammaCorrection(bool disable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::EnableAlphaToCoverage(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::SetShadowDepthFiltering(Sampler_t stage)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::BlendOp(ShaderBlendOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderShadow::BlendOpSeparateAlpha(ShaderBlendOp_t op)
{
	NOT_IMPLEMENTED_FUNC();
}
