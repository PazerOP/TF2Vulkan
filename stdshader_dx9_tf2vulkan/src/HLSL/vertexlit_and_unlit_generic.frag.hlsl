#include "common_ps_fxc.hlsli"

struct PS_INPUT
{
	[[vk::location(LOCATION_POSITION0)]] float4 projPos              : POSITION;
	[[vk::location(LOCATION_FOG)]]       float fog                   : FOG;

	[[vk::location(LOCATION_TEXCOORD0)]] float3 baseTexCoord         : TEXCOORD0;
	[[vk::location(LOCATION_TEXCOORD1)]] float3 detailTexCoord       : TEXCOORD1;
	[[vk::location(LOCATION_TEXCOORD2)]] float4 color                : TEXCOORD2;

	[[vk::location(LOCATION_TEXCOORD3)]] float3 worldVertToEyeVector : TEXCOORD3;
	[[vk::location(LOCATION_TEXCOORD4)]] float3 worldSpaceNormal     : TEXCOORD4;

	[[vk::location(LOCATION_TEXCOORD6)]] float4 vProjPos             : TEXCOORD6;
	[[vk::location(LOCATION_TEXCOORD7)]] float4 worldPos_ProjPosZ    : TEXCOORD7;
	[[vk::location(LOCATION_COLOR1)]]    float4 fogFactorW           : COLOR1;
	[[vk::location(LOCATION_COLOR0)]]    float3 SeamlessWeights      : COLOR0;
};

[[vk::binding(0)]] Texture2D BaseTexture;
[[vk::binding(1)]] SamplerState BaseTextureSampler;

float4 main(const PS_INPUT i) : SV_Target
{
	return BaseTexture.Sample(BaseTextureSampler, i.baseTexCoord.xy);
}
