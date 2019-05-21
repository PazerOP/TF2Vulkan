#ifndef XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
#define XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD

[[vk::binding(BINDING_CBUF_SHADERCUSTOM)]] cbuffer ShaderCustomConstants
{
	float4 cBaseTexCoordTransform[2];
	float4 cDetailTexCoordTransform[2];
	float4 cBumpTexCoordTransform[2];

	float g_CloakFactor;
	float g_RefractAmount;
	float4 g_RefractColorTint;

	float4x2 g_ViewProjR01;

	float4 cMorphSubrect;
	float3 cMorphTargetTextureDim;
	float cSeamlessScale;
	float4x4 g_FlashlightWorldToTexture;

	float g_fVertexAlpha;
};

[[vk::constant_id(SPEC_CONST_ID_BASE + 1)]] const bool TEXACTIVE_BASETEXTURE = false;
[[vk::constant_id(SPEC_CONST_ID_BASE + 2)]] const bool TEXACTIVE_BUMPMAP = false;

[[vk::binding(0)]] Texture2D BaseTexture;
[[vk::binding(0)]] SamplerState BaseTextureSampler;

[[vk::binding(1)]] Texture2D BumpMapTexture;
[[vk::binding(1)]] SamplerState BumpMapTextureSampler;

[[vk::binding(2)]] Texture2D morphTexture;
[[vk::binding(2)]] SamplerState morphSampler;

#endif // XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
