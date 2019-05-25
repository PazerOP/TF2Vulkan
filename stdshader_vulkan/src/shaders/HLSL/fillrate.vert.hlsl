#include "common_vs_fxc.hlsli"

struct VS_INPUT
{
	float4 vPos				: POSITION;
	float3 vBoneWeights		: BLENDWEIGHT;
	uint3  vBoneIndices		: BLENDINDICES;
};

struct VS_OUTPUT
{
	float4 vProjPos		: POSITION;
};

VS_OUTPUT main(const VS_INPUT v)
{
	VS_OUTPUT o = (VS_OUTPUT)0;

	float3 worldPos;
	SkinPosition(SKINNING, v.vPos, v.vBoneWeights, v.vBoneIndices, worldPos);

	o.vProjPos = mul(float4(worldPos, 1), cViewProj);

#ifdef INVERT_Y
	o.vProjPos.y = 1 - o.vProjPos.y;
#endif

	return o;
}
