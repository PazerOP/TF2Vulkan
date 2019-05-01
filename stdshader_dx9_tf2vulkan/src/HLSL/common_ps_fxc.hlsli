#ifndef INCLUDE_GUARD_COMMON_PS_FXC_HLSLI
#define INCLUDE_GUARD_COMMON_PS_FXC_HLSLI

#include "common_fxc.hlsli"

cbuffer PixelShaderStandardConstants
{
	float4 g_LinearFogColor;
	float4 cLightsScale;
	float4 cFlashlightColor;
	float4 cFlashlightScreenScale;
};

#endif // INCLUDE_GUARD_COMMON_PS_FXC_HLSLI
