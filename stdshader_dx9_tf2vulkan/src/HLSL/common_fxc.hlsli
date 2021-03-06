#ifndef INCLUDE_GUARD_COMMON_FXC_HLSLI
#define INCLUDE_GUARD_COMMON_FXC_HLSLI

#include "../../include/stdshader_dx9_tf2vulkan/ShaderDataShared.h"

#define VTX_SHADER_START_ID 1024
#define PIX_SHADER_START_ID 2048

[[vk::constant_id(1)]] const bool VERTEXCOLOR = false;
[[vk::constant_id(2)]] const bool CUBEMAP = false;
[[vk::constant_id(3)]] const bool HALFLAMBERT = false;
[[vk::constant_id(4)]] const bool FLASHLIGHT = false;
[[vk::constant_id(5)]] const bool SEAMLESS_BASE = false;
[[vk::constant_id(6)]] const bool SEAMLESS_DETAIL = false;
[[vk::constant_id(7)]] const bool SEPARATE_DETAIL_UVS = false;
[[vk::constant_id(8)]] const bool DECAL = false;
[[vk::constant_id(9)]] const bool DONT_GAMMA_CONVERT_VERTEX_COLOR = false;

[[vk::constant_id(10)]] const bool DYNAMIC_LIGHT = false;
[[vk::constant_id(11)]] const bool STATIC_LIGHT_VERTEX = false;
[[vk::constant_id(12)]] const bool STATIC_LIGHT_LIGHTMAP = false;
[[vk::constant_id(13)]] const int DOWATERFOG = 0;
[[vk::constant_id(14)]] const bool SKINNING = false;
[[vk::constant_id(15)]] const bool LIGHTING_PREVIEW = false;
[[vk::constant_id(16)]] const bool MORPHING = false;
[[vk::constant_id(17)]] const bool NORMALMAPPING = false;

[[vk::constant_id(18)]] const bool DIFFUSELIGHTING = false;
[[vk::constant_id(19)]] const bool SELFILLUM = false;

static const int BINDING_CBUF_VS_STANDARD = 10;
static const int BINDING_CBUF_VS_CUSTOM = 11;

static const int BINDING_CBUF_PS_STANDARD = 20;
static const int BINDING_CBUF_PS_CUSTOM = 21;

static const int BINDING_BASE_SAMPLER = 100;
static const int BINDING_BASE_TEXTURE = 200;

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
