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
	output.texCoord = input.texCoord;
	//output.texCoord.y = output.texCoord.y;

	return output;
}
