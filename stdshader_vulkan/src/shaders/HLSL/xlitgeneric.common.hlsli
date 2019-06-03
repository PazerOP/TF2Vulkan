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

	float3 baseTexCoord         : TEXCOORD0;
	float3 detailTexCoord       : TEXCOORD1;

	float3 worldVertToEyeVector : TEXCOORD3;
	float3 worldSpaceNormal     : TEXCOORD4;
	float4 worldSpaceTangent    : TANGENT;
	float3 worldSpaceBinormal   : BINORMAL;
	float4 lightAtten           : LIGHT_ATTEN;

	float4 vProjPos             : POS_PROJ;
	float3 worldSpacePos        : POS_WORLD;

	float3 SeamlessWeights      : TEXCOORD8;

	float3 boneWeightsOut       : TEST_BONEWEIGHTSOUT;

	float1 fog : FOG;
	float4 fogFactorW           : TEXCOORD9;
};

#ifdef VERTEX_SHADER
#define VS_OUTPUT VS_TO_PS_INTERSTAGE_DATA
#elif defined(PIXEL_SHADER) || defined(FRAGMENT_SHADER)
#define PS_INPUT VS_TO_PS_INTERSTAGE_DATA
#else
#error "Either VERTEX_SHADER or PIXEL_SHADER must be defined"
#endif

[[vk::constant_id(SPEC_CONST_ID_BASE + 1)]] const bool AMBIENT_LIGHT = false;

#if 1 // TODO: Array-based textures
//[[vk::constant_id(SPEC_CONST_ID_BASE + 7)]]  const uint TEXINDEX_BASETEXTURE = 0;
//[[vk::constant_id(SPEC_CONST_ID_BASE + 8)]]  const uint SMPINDEX_BASETEXTURE = 0;

[[vk::constant_id(SPEC_CONST_ID_BASE + 9)]]  const uint TEXINDEX_BUMPMAP = 0;
[[vk::constant_id(SPEC_CONST_ID_BASE + 10)]] const uint SMPINDEX_BUMPMAP = 0;

[[vk::constant_id(SPEC_CONST_ID_BASE + 11)]] const uint TEXINDEX_MORPH = 0;
[[vk::constant_id(SPEC_CONST_ID_BASE + 12)]] const uint SMPINDEX_MORPH = 0;

[[vk::constant_id(SPEC_CONST_ID_BASE + 13)]] const uint TEXINDEX_LIGHTWARP = 0;
[[vk::constant_id(SPEC_CONST_ID_BASE + 14)]] const uint SMPINDEX_LIGHTWARP = 0;

//#define BaseTexture g_Textures2D[TEXINDEX_BASETEXTURE]
//#define BaseTextureSampler g_Samplers[SMPINDEX_BASETEXTURE]
DEFINE_TEX2D(2, BASE);

#define BumpMapTexture g_Textures2D[TEXINDEX_BUMPMAP]
#define BumpMapTextureSampler g_Samplers[SMPINDEX_BUMPMAP]

#define morphTexture g_Textures2D[TEXINDEX_MORPH]
#define morphSampler g_Samplers[SMPINDEX_MORPH]

#define LightWarpTexture g_Textures2D[TEXINDEX_LIGHTWARP]
#define LightWarpTextureSampler g_Samplers[SMPINDEX_LIGHTWARP]

//#define TEXACTIVE_BASETEXTURE (TEXINDEX_BASETEXTURE > 0)
#define TEXACTIVE_BUMPMAP (TEXINDEX_BUMPMAP > 0)
#define TEXACTIVE_MORPH (TEXINDEX_MORPH > 0)
#define TEXACTIVE_LIGHTWARP (TEXINDEX_LIGHTWARP > 0)

#else
// Texture2D BaseTexture : register(t0);
// SamplerState BaseTextureSampler : register(s0);

// Texture2D BumpMapTexture : register(t1);
// SamplerState BumpMapTextureSampler : register(s1);

// Texture2D morphTexture : register(t2);
// SamplerState morphSampler : register(s2);

// Texture2D LightWarpTexture : register(t3);
// SamplerState LightWarpTextureSampler : register(s3);
#endif

#endif // XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
