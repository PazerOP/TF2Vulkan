#include "common_ps_fxc.hlsli"

struct PS_INPUT
{
	float4 projPos              : POSITION;
	float fog                   : FOG;

	float3 baseTexCoord         : TEXCOORD0;
	float3 detailTexCoord       : TEXCOORD1;
	float4 color                : TEXCOORD2;

	float3 worldVertToEyeVector : TEXCOORD3;
	float3 worldSpaceNormal     : TEXCOORD4;

	float4 vProjPos             : TEXCOORD6;
	float4 worldPos_ProjPosZ    : TEXCOORD7;
	float3 SeamlessWeights      : COLOR0;
	float4 fogFactorW           : COLOR1;
};

[[vk::binding(0)]] Texture2D BaseTexture;
[[vk::binding(1)]] SamplerState BaseTextureSampler;

float4 main(const PS_INPUT i) : SV_Target
{
	return BaseTexture.Sample(BaseTextureSampler, i.baseTexCoord.xy);
}
