#include "common_ps_fxc.hlsli"
#include "bufferclearobeystencil.common.hlsli"

struct PS_INPUT
{
	float4 vColor : COLOR;
};

float4 main(const PS_INPUT i) : SV_TARGET
{
	if (USESCOLOR)
		return i.vColor;
	else
		return float4(0, 0, 0, 0);
}
