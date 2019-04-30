#ifndef INCLUDE_GUARD_COMMON_FXC_HLSLI
#define INCLUDE_GUARD_COMMON_FXC_HLSLI

[[vk::constant_id(1)]] const bool VERTEXCOLOR = false;
[[vk::constant_id(2)]] const bool CUBEMAP = false;
[[vk::constant_id(3)]] const bool HALFLAMBERT = false;
[[vk::constant_id(4)]] const bool FLASHLIGHT = false;
[[vk::constant_id(5)]] const bool SEAMLESS_BASE = false;
[[vk::constant_id(6)]] const bool SEAMLESS_DETAIL = false;
[[vk::constant_id(7)]] const bool SEAMLESS_DETAIL_UVS = false;
[[vk::constant_id(8)]] const bool DECAL = false;
[[vk::constant_id(9)]] const bool DONT_GAMMA_CONVERT_VERTEX_COLOR = false;

[[vk::constant_id(10)]] const bool DYNAMIC_LIGHT = false;
[[vk::constant_id(11)]] const bool STATIC_LIGHT_VERTEX = false;
[[vk::constant_id(12)]] const bool STATIC_LIGHT_LIGHTMAP = false;
[[vk::constant_id(13)]] const int DOWATERFOG = 0;
[[vk::constant_id(14)]] const bool SKINNING = false;
[[vk::constant_id(15)]] const bool LIGHTING_PREVIEW = false;
[[vk::constant_id(16)]] const bool MORPHING = false;

#define g_FogType DOWATERFOG

// These cause bad codegen with glslangValidator
#define sampler2D "FIXME"

float3 mul3x3(float3 v, float3x3 m)
{
	return mul(v, m);
}
float3 mul4x3(float4 v, float4x3 m)
{
	return mul(v, m);
}

float3 LinearToGamma(const float3 f3linear)
{
	return pow(f3linear, 1.0f / 2.2f);
}

float4 LinearToGamma(const float4 f4linear)
{
	return float4(pow(f4linear.xyz, 1.0f / 2.2f), f4linear.w);
}

float LinearToGamma(const float f1linear)
{
	return pow(f1linear, 1.0f / 2.2f);
}

float3 GammaToLinear(const float3 gamma)
{
	return pow(gamma, 2.2f);
}

float4 GammaToLinear(const float4 gamma)
{
	return float4(pow(gamma.xyz, 2.2f), gamma.w);
}

#endif // INCLUDE_GUARD_COMMON_FXC_HLSLI
