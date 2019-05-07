#include "common_vs_fxc.hlsli"

struct VS_INPUT
{
	float4 pos : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

VS_OUTPUT main( const VS_INPUT input )
{
	VS_OUTPUT output;

	output.pos = input.pos;
	output.pos.z = 0.5f;

	output.pos.xyz *= 0.75f;
	output.texCoord = input.texCoord;

	return output;
}
