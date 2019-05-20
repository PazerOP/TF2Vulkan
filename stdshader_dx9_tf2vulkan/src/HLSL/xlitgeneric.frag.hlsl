#include "common_ps_fxc.hlsli"
#include "xlitgeneric.common.hlsli"

struct PS_INPUT
{
	//float4 projPos              : SV_Position;
	float4 color                : TEXCOORD2;
	float fog                   : FOG;

	float3 baseTexCoord         : TEXCOORD0;
	float3 detailTexCoord       : TEXCOORD1;

	float3 worldVertToEyeVector : TEXCOORD3;
	float3 worldSpaceNormal     : TEXCOORD4;

	float4 vProjPos             : TEXCOORD6;
	float4 worldPos_ProjPosZ    : TEXCOORD7;
	float3 SeamlessWeights      : COLOR0;
	float4 fogFactorW           : COLOR1;
};

float4 main(const PS_INPUT i) : SV_Target
{
	float3 diffuseColor = (float3)1;
	if (DIFFUSELIGHTING || VERTEXCOLOR)
		diffuseColor = i.color.bgr; // FIXME REALLY SOON: Why is this bgr????

	float4 baseTextureColor = (float4)1;
	if (TEXACTIVE_BASETEXTURE)
		baseTextureColor = BaseTexture.Sample(BaseTextureSampler, i.baseTexCoord.xy);

	if (DIFFUSELIGHTING)
		return baseTextureColor;

	const float3 finalColor = diffuseColor.rgb * baseTextureColor.rgb;
	const float finalAlpha = lerp(baseTextureColor.a, baseTextureColor.a * i.color.a, g_fVertexAlpha);
	return float4(finalColor, finalAlpha);
}
