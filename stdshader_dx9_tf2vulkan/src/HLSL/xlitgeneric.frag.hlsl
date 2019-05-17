#include "common_ps_fxc.hlsli"
#include "xlitgeneric.common.hlsli"

struct PS_INPUT
{
	//float4 projPos              : SV_Position;
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

float4 main(const PS_INPUT i) : SV_Target
{
	float4 color = (float4)1;

	if (VERTEXCOLOR)
		color *= i.color;

	color *= BaseTexture.Sample(BaseTextureSampler, i.baseTexCoord.xy);

	return color;
}
