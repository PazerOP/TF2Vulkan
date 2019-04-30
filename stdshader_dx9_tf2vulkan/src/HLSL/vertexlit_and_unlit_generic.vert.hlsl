#include "common_vs_fxc.hlsli"

cbuffer VertexShaderCustomConstants
{
	float4 cBaseTexCoordTransform[2];
	float cSeamlessScale;
	float4 cDetailTexCoordTransform[2];

	float3 cMorphTargetTextureDim;
	float4 cMorphSubrect;
};

SamplerState morphSampler;
Texture2D morphTexture;

#define g_bDecalOffset (MORPHING && DECAL)
#define g_bSkinning (SKINNING)

struct VS_INPUT
{
	// This is all of the stuff that we ever use.
	float4 vPos             : POSITION;
	float4 vBoneWeights     : BLENDWEIGHT;
	float4 vBoneIndices     : BLENDINDICES;
	float4 vNormal          : NORMAL;
	float4 vColor           : COLOR0;
	float3 vSpecular        : COLOR1;
	// make these float2's and stick the [n n 0 1] in the dot math.
	float4 vTexCoord0       : TEXCOORD0;
	float4 vTexCoord1       : TEXCOORD1;
	float4 vTexCoord2       : TEXCOORD2;
	float4 vTexCoord3       : TEXCOORD3;

	// Position and normal/tangent deltas
	float3 vPosFlex         : POSITION1;
	float3 vNormalFlex      : NORMAL1;
	float vVertexID         : POSITION2;
};

struct VS_OUTPUT
{
	float4 projPos              : POSITION;
	float fog                   : FOG;

	float3 baseTexCoord         : TEXCOORD0;
	float3 detailTexCoord       : TEXCOORD1;
	float4 color                : TEXCOORD2;

	float3 worldVertToEyeVector : TEXCOORD3;
	float3 worldSpaceNormal     : TEXCOORD4;

	float4 vProjPos             : TEXCOORD6;
	float4 worldPos_ProjPosZ    : TEXCOORD7;
	float4 fogFactorW           : COLOR1;
	float3 SeamlessWeights      : COLOR0;
};

VS_OUTPUT main( const VS_INPUT v )
{
	VS_OUTPUT o;

	float4 vPosition = v.vPos;
	float3 vNormal = 0;
	if ((!VERTEXCOLOR && (DYNAMIC_LIGHT || STATIC_LIGHT_VERTEX)) ||
		FLASHLIGHT ||
		SEAMLESS_BASE ||
		SEAMLESS_DETAIL ||
		LIGHTING_PREVIEW ||
		g_bDecalOffset ||
		CUBEMAP)
	{
		DecompressVertex_Normal(v.vNormal, vNormal);
	}

	float3 NNormal;
	if (SEAMLESS_BASE || SEAMLESS_DETAIL)
	{
		NNormal = normalize(vNormal);
		o.SeamlessWeights.xyz = NNormal * NNormal;
	}

	if (!MORPHING)
		ApplyMorph(v.vPosFlex, v.vNormalFlex, vPosition.xyz, vNormal);
	else
	{
		ApplyMorph(morphTexture, morphSampler, cMorphTargetTextureDim, cMorphSubrect,
			v.vVertexID, v.vTexCoord2, vPosition.xyz, vNormal);
	}

	// Perform skinning
	float3 worldNormal, worldPos;
	SkinPositionAndNormal(
		g_bSkinning,
		vPosition, vNormal,
		v.vBoneWeights, v.vBoneIndices,
		worldPos, worldNormal);

	if (!VERTEXCOLOR)
		worldNormal = normalize(worldNormal);

	if (MORPHING && DECAL)
		worldPos += worldNormal * 0.05f * v.vTexCoord2.z;

	o.worldSpaceNormal = worldNormal;

	// Transform into projection space
	float4 vProjPos = mul(float4(worldPos, 1), cViewProj);
	o.projPos = vProjPos;
	vProjPos.z = dot(float4(worldPos, 1), cViewProjZ);

	o.vProjPos = vProjPos;
	o.fogFactorW.w = CalcFog(worldPos, vProjPos, DOWATERFOG);
	o.fog = o.fogFactorW.w;
	o.worldPos_ProjPosZ.xyz = worldPos.xyz;
	o.worldPos_ProjPosZ.w = vProjPos.z;

	// Needed for cubemaps
	if (CUBEMAP)
		o.worldVertToEyeVector.xyz = cEyePos - worldPos;


#if FLASHLIGHT
	o.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
#else
	if (VERTEXCOLOR)
	{
		// Assume that this is unlitgeneric if you are using vertex color.
		o.color.rgb = (DONT_GAMMA_CONVERT_VERTEX_COLOR) ? v.vColor.rgb : GammaToLinear(v.vColor.rgb);
		o.color.a = v.vColor.a;
	}
	else
	{
		o.color.xyz = DoLighting(worldPos, worldNormal, v.vSpecular, STATIC_LIGHT_VERTEX, DYNAMIC_LIGHT, HALFLAMBERT);
	}
#endif


#if SEAMLESS_BASE
	o.SeamlessTexCoord.xyz = SEAMLESS_SCALE * v.vPos.xyz;
#else
	// Base texture coordinates
	o.baseTexCoord.x = dot(v.vTexCoord0, cBaseTexCoordTransform[0]);
	o.baseTexCoord.y = dot(v.vTexCoord0, cBaseTexCoordTransform[1]);
#endif

#if SEAMLESS_DETAIL
	// FIXME: detail texcoord as a 2d xform doesn't make much sense here, so I just do enough so
	// that scale works. More smartness could allow 3d xform.
	o.SeamlessDetailTexCoord.xyz = (SEAMLESS_SCALE * cDetailTexCoordTransform[0].x) * v.vPos.xyz;
#else
	// Detail texture coordinates
	// FIXME: This shouldn't have to be computed all the time.
	o.detailTexCoord.x = dot(v.vTexCoord0, cDetailTexCoordTransform[0]);
	o.detailTexCoord.y = dot(v.vTexCoord0, cDetailTexCoordTransform[1]);
#endif

#if SEPARATE_DETAIL_UVS
	o.detailTexCoord.xy = v.vTexCoord1.xy;
#endif

#if LIGHTING_PREVIEW
	float dot = 0.5 + 0.5 * worldNormal * float3(0.7071, 0.7071, 0);
	o.color.xyz = float3(dot, dot, dot);
#endif

#if defined ( _X360 ) && FLASHLIGHT
	o.flashlightSpacePos = mul(float4(worldPos, 1.0f), g_FlashlightWorldToTexture);
#endif

	return o;
}
