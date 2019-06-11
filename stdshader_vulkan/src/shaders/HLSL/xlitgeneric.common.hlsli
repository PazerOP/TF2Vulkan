#ifndef XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
#define XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD

#include "ShaderComponentData.hlsli"

struct XLitGenericUniforms
{
	BumpmapUniforms bump;
	DetailUniforms detail;
	BaseUniforms base;
	LightmapUniforms lightmap;

	struct
	{
		float1 vertexAlpha;
	} xlit;
};

[[vk::binding(BINDING_CBUF_SHADERCUSTOM)]] ConstantBuffer<XLitGenericUniforms> cCustom;

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

	float1 fog                  : FOG;
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

DEFINE_TEX2D(2, BASETEXTURE, TEX_DEFAULT_COLOR_WHITE);
//[[vk::constant_id(SPEC_CONST_ID_BASE + 2)]] const int TEXINDEX_BASETEXTURE = TEX_DEFAULT_COLOR_WHITE;
//[[vk::constant_id(SPEC_CONST_ID_BASE + 3)]] const int SMPINDEX_BASETEXTURE = -1;
DEFINE_TEX2D(4, BUMPMAP, TEX_DEFAULT_COLOR_FLATNORMAL);
DEFINE_TEX2D(6, MORPH, TEX_DEFAULT_COLOR_BLACK);
DEFINE_TEX2D(8, LIGHTWARP, TEX_DEFAULT_COLOR_WHITE);

DEFINE_TEX2D(10, LIGHTMAP1, TEX_DEFAULT_COLOR_GREY50);
DEFINE_TEX2D(12, LIGHTMAP2, TEX_DEFAULT_COLOR_GREY50);
DEFINE_TEX2D(14, LIGHTMAP3, TEX_DEFAULT_COLOR_GREY50);

#define TEXACTIVE_BASETEXTURE (IsTextureActive(TEXINDEX_BASETEXTURE))
#define TEXACTIVE_BUMPMAP (IsTextureActive(TEXINDEX_BUMPMAP))
#define TEXACTIVE_MORPH (IsTextureActive(TEXINDEX_MORPH))
#define TEXACTIVE_LIGHTWARP (IsTextureActive(TEXINDEX_LIGHTWARP))

#endif // XLITGENERIC_COMMON_HLSLI_INCLUDE_GUARD
