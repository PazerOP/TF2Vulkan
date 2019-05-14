#ifndef INCLUDE_GUARD_COMMON_PS_FXC_HLSLI
#define INCLUDE_GUARD_COMMON_PS_FXC_HLSLI

#include "common_fxc.hlsli"

cbuffer PixelShaderStandardConstants
{
	float4 g_LinearFogColor;
	float4 cLightScale;
	float4 cFlashlightColor;
	float4 cFlashlightScreenScale;
};

#define OO_DESTALPHA_DEPTH_RANGE (g_LinearFogColor.w)

#define LINEAR_LIGHT_SCALE (cLightScale.x)
#define LIGHT_MAP_SCALE (cLightScale.y)
#define ENV_MAP_SCALE (cLightScale.z)
#define GAMMA_LIGHT_SCALE (cLightScale.w)

static const int TONEMAP_SCALE_NONE = 0;
static const int TONEMAP_SCALE_LINEAR = 1;
static const int TONEMAP_SCALE_GAMMA = 2;

static const int PIXEL_FOG_TYPE_NONE = -1;
static const int PIXEL_FOG_TYPE_RANGE = 0;
static const int PIXEL_FOG_TYPE_HEIGHT = 1;

float SoftParticleDepth(float flDepth)
{
	return flDepth * OO_DESTALPHA_DEPTH_RANGE;
}

float DepthToDestAlpha(const float flProjZ)
{
	return SoftParticleDepth(flProjZ);
}

float3 BlendPixelFog(const float3 vShaderColor, float pixelFogFactor, const float3 vFogColor, const int iPIXELFOGTYPE)
{
	if (iPIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE) //either range fog or no fog depending on fog parameters and whether this is ps20 or ps2b
	{
		//squaring the factor will get the middle range mixing closer to hardware fog
		return lerp(vShaderColor.rgb, vFogColor.rgb, pixelFogFactor * pixelFogFactor);
	}
	else if (iPIXELFOGTYPE == PIXEL_FOG_TYPE_HEIGHT)
	{
		return lerp(vShaderColor.rgb, vFogColor.rgb, pixelFogFactor);
	}
	else if (iPIXELFOGTYPE == PIXEL_FOG_TYPE_NONE)
	{
		return vShaderColor;
	}
}

float4 FinalOutput(const float4 vShaderColor, float pixelFogFactor, const int iPIXELFOGTYPE,
	const int iTONEMAP_SCALE_TYPE, const bool bWriteDepthToDestAlpha = false, const float flProjZ = 1.0f)
{
	float4 result;
	if (iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_LINEAR)
	{
		result.rgb = vShaderColor.rgb * LINEAR_LIGHT_SCALE;
	}
	else if (iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_GAMMA)
	{
		result.rgb = vShaderColor.rgb * GAMMA_LIGHT_SCALE;
	}
	else if (iTONEMAP_SCALE_TYPE == TONEMAP_SCALE_NONE)
	{
		result.rgb = vShaderColor.rgb;
	}

	if (bWriteDepthToDestAlpha)
		result.a = DepthToDestAlpha(flProjZ);
	else
		result.a = vShaderColor.a;

	if (iPIXELFOGTYPE == PIXEL_FOG_TYPE_RANGE)
	{
		result.rgb = BlendPixelFog(result.rgb, pixelFogFactor, g_LinearFogColor.rgb, iPIXELFOGTYPE);
	}


	return result;
}

#endif // INCLUDE_GUARD_COMMON_PS_FXC_HLSLI
