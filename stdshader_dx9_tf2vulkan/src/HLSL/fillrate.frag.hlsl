//========== Copyright (c) Valve Corporation, All rights reserved. ==========//

#include "common_ps_fxc.hlsli"

cbuffer PixelShaderCustomConstants
{
	float4 g_ColorConstant;
};

float4 main() : COLOR
{
	float4 result = (g_ColorConstant * (1.0 / 2.0));

	return FinalOutput(result, 0, PIXEL_FOG_TYPE_NONE, TONEMAP_SCALE_NONE);
}
