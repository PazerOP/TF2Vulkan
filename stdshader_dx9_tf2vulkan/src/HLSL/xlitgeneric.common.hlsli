#ifndef XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
#define XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD

cbuffer ShaderCustomConstants
{
	float4 cBaseTexCoordTransform[2];
	float4 cDetailTexCoordTransform[2];
	float4 cBumpTexCoordTransform[2];

	float4 cMorphSubrect;
	float3 cMorphTargetTextureDim;
	float cSeamlessScale;
	float4x4 g_FlashlightWorldToTexture;
};

[[vk::binding(0)]] Texture2D BaseTexture;
[[vk::binding(0)]] SamplerState BaseTextureSampler;

[[vk::binding(1)]] Texture2D BumpMapTexture;
[[vk::binding(1)]] SamplerState BumpMapTextureSampler;

[[vk::binding(2)]] SamplerState morphSampler;
[[vk::binding(2)]] Texture2D morphTexture;

#endif // XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
