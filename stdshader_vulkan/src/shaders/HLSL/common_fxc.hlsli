#ifndef INCLUDE_GUARD_COMMON_FXC_HLSLI
#define INCLUDE_GUARD_COMMON_FXC_HLSLI

#include "../../../include/stdshader_vulkan/ShaderDataShared.h"

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
[[vk::constant_id(9)]] const bool GAMMA_CONVERT_VERTEX_COLOR = true;

[[vk::constant_id(10)]] const bool DYNAMIC_LIGHT = false;
[[vk::constant_id(11)]] const bool STATIC_LIGHT_VERTEX = false;
[[vk::constant_id(12)]] const bool STATIC_LIGHT_LIGHTMAP = false;
[[vk::constant_id(13)]] const int DOWATERFOG = 0;
[[vk::constant_id(14)]] const bool SKINNING = false;
[[vk::constant_id(16)]] const bool MORPHING = false;
[[vk::constant_id(17)]] const bool NORMALMAPPING = false;

[[vk::constant_id(18)]] const bool DIFFUSELIGHTING = false;
[[vk::constant_id(19)]] const bool SELFILLUM = false;

[[vk::constant_id(20)]] const uint TEXTURE2D_COUNT = 1;
[[vk::constant_id(21)]] const uint SAMPLER_COUNT = 1;

static const uint SPEC_CONST_ID_BASE = 22;

[[vk::binding(BINDING_TEX2D)]] Texture2D g_Textures2D[TEXTURE2D_COUNT];
[[vk::binding(BINDING_SAMPLERS)]] SamplerState g_Samplers[SAMPLER_COUNT];

static const float cOverbright = 2.0f;
static const float cOOOverbright = 1.0f / cOverbright;
static const float cOneThird = 1.0f / 3.0f;

#define DEFINE_TEX2D(baseSpecConstIndex, nameCaps) \
	[[vk::constant_id(SPEC_CONST_ID_BASE + baseSpecConstIndex + 0)]] const uint TEXINDEX_ ## nameCaps = 0; \
	[[vk::constant_id(SPEC_CONST_ID_BASE + baseSpecConstIndex + 1)]] const uint SMPINDEX_ ## nameCaps = 0; \
	static const bool TEXACTIVE_ ## nameCaps ## TEXTURE = (TEXINDEX_ ## nameCaps) > 0; \
	static const Texture2D nameCaps ## TEXTURE = g_Textures2D[TEXINDEX_ ## nameCaps]; \
	static const SamplerState nameCaps ## TEXTURE_SAMPLER = g_Samplers[SMPINDEX_ ## nameCaps];

[[vk::binding(BINDING_CBUF_SHADERCOMMON)]] cbuffer ShaderCommonConstants
{
	// VS standard
	float4x4 cModelViewProj;
	float4x4 cViewProj;

	float cOOGamma;
	uint g_nLightCountRegister;
	bool4 g_bLightEnabled;

	float3 cEyePos;
	float cWaterZ;

	float4 cFlexScale;

	FogParams cFogParams;

	LightInfo cLightInfo[4];
	AmbientLightCube cAmbientCube;

	// PS standard
	float4 g_LinearFogColor;
	float4 cLightScale;
};

#define NUM_LIGHTS g_nLightCountRegister

#define g_FogType DOWATERFOG

// These cause bad codegen with glslangValidator
#define tex1D "FIXME"
#define tex2D "FIXME"
#define tex3D "FIXME"
#define sampler1D "FIXME"
#define sampler2D "FIXME"
#define sampler3D "FIXME"

// This just shouldn't be used anymore
#define D3DCOLORtoUBYTE4 "FIXME"

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

float3 Vec3WorldToTangent(float3 iWorldVector, float3 iWorldNormal, float3 iWorldTangent, float3 iWorldBinormal)
{
	float3 vTangentVector;
	vTangentVector.x = dot(iWorldVector.xyz, iWorldTangent.xyz);
	vTangentVector.y = dot(iWorldVector.xyz, iWorldBinormal.xyz);
	vTangentVector.z = dot(iWorldVector.xyz, iWorldNormal.xyz);
	return vTangentVector.xyz; // Return without normalizing
}

float3 Vec3WorldToTangentNormalized(float3 iWorldVector, float3 iWorldNormal, float3 iWorldTangent, float3 iWorldBinormal)
{
	return normalize(Vec3WorldToTangent(iWorldVector, iWorldNormal, iWorldTangent, iWorldBinormal));
}

float3 Vec3TangentToWorld(float3 iTangentVector, float3 iWorldNormal, float3 iWorldTangent, float3 iWorldBinormal)
{
	float3 vWorldVector;
	vWorldVector.xyz = iTangentVector.x * iWorldTangent.xyz;
	vWorldVector.xyz += iTangentVector.y * iWorldBinormal.xyz;
	vWorldVector.xyz += iTangentVector.z * iWorldNormal.xyz;
	return vWorldVector.xyz; // Return without normalizing
}

float3 Vec3TangentToWorldNormalized(float3 iTangentVector, float3 iWorldNormal, float3 iWorldTangent, float3 iWorldBinormal)
{
	return normalize(Vec3TangentToWorld(iTangentVector, iWorldNormal, iWorldTangent, iWorldBinormal));
}

#endif // INCLUDE_GUARD_COMMON_FXC_HLSLI
