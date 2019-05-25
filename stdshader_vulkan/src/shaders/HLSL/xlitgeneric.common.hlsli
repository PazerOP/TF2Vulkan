#ifndef XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
#define XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD

[[vk::binding(BINDING_CBUF_SHADERCUSTOM)]] cbuffer ShaderCustomConstants
{
	float4 cBaseTexCoordTransform[2];
	float4 cDetailTexCoordTransform[2];
	float4 cBumpTexCoordTransform[2];

	float g_CloakFactor;
	float g_RefractAmount;
	float4 g_RefractColorTint;

	float4x2 g_ViewProjR01;

	float4 cMorphSubrect;
	float3 cMorphTargetTextureDim;
	float cSeamlessScale;
	float4x4 g_FlashlightWorldToTexture;

	float g_fVertexAlpha;
};

struct VS_TO_PS_INTERSTAGE_DATA
{
#ifdef VERTEX_SHADER
	float4 projPos              : SV_Position;
#endif

	float4 color                : COLOR0;
	float fog : FOG;

	float3 baseTexCoord         : TEXCOORD0;
	float3 detailTexCoord       : TEXCOORD1;

	float3 worldVertToEyeVector : TEXCOORD3;
	float3 worldSpaceNormal     : TEXCOORD4;
	float4 worldSpaceTangent;
	float3 worldSpaceBinormal;
	float4 lightAtten;

	float4 vProjPos             : TEXCOORD6;
	float4 worldPos_ProjPosZ    : TEXCOORD7;

	float3 SeamlessWeights      : TEXCOORD8;
	float4 fogFactorW           : TEXCOORD9;

	float3 boneWeightsOut       : TEST_BONEWEIGHTSOUT;
};

#ifdef VERTEX_SHADER
#define VS_OUTPUT VS_TO_PS_INTERSTAGE_DATA
#elif defined(PIXEL_SHADER) || defined(FRAGMENT_SHADER)
#define PS_INPUT VS_TO_PS_INTERSTAGE_DATA
#else
#error "Either VERTEX_SHADER or PIXEL_SHADER must be defined"
#endif

static int SPEC_CONST_ID_NEXT = SPEC_CONST_ID_BASE + 1;
[[vk::constant_id(SPEC_CONST_ID_NEXT++)]] const bool AMBIENT_LIGHT = false;
[[vk::constant_id(SPEC_CONST_ID_BASE + 1)]] const bool TEXACTIVE_BASETEXTURE = false;
[[vk::constant_id(SPEC_CONST_ID_BASE + 2)]] const bool TEXACTIVE_BUMPMAP = false;
[[vk::constant_id(SPEC_CONST_ID_BASE + 3)]] const bool TEXACTIVE_LIGHTWARP = false;

#if false // TODO: Array-based textures
[[vk::constant_id(SPEC_CONST_ID_BASE + 3)]] const uint TEXTURE_COUNT = 1;
[[vk::constant_id(SPEC_CONST_ID_BASE + 4)]] const uint SAMPLER_COUNT = 1;

[[vk::constant_id(SPEC_CONST_ID_BASE + 5)]] const uint TEXINDEX_BASE = 0;
[[vk::constant_id(SPEC_CONST_ID_BASE + 5)]] const uint SMPINDEX_BASE = 0;

[[vk::binding(100)]] Texture2D g_Textures[TEXTURE_COUNT];
[[vk::binding(100)]] SamplerState g_Samplers[SAMPLER_COUNT];
#endif

[[vk::binding(0)]] Texture2D BaseTexture;
[[vk::binding(0)]] SamplerState BaseTextureSampler;

[[vk::binding(1)]] Texture2D BumpMapTexture;
[[vk::binding(1)]] SamplerState BumpMapTextureSampler;

[[vk::binding(2)]] Texture2D morphTexture;
[[vk::binding(2)]] SamplerState morphSampler;

[[vk::binding(3)]] Texture2D LightWarpTexture;
[[vk::binding(3)]] SamplerState LightWarpTextureSampler;

#endif // XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
