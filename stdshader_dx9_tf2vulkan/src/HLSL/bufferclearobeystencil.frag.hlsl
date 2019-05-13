#include "common_ps_fxc.hlsli"

struct PS_INPUT
{
	float4 vColor : COLOR;
};

float4 main(const PS_INPUT i) : SV_TARGET
{
	return i.vColor;
}
