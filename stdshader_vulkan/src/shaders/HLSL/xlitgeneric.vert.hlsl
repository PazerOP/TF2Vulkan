#include "common_vs_fxc.hlsli"

#define VERTEX_SHADER
#include "xlitgeneric.common.hlsli"

#define SEAMLESS_SCALE (cSeamlessScale.x)

#define g_bDecalOffset (MORPHING && DECAL)
#define g_bSkinning (SKINNING)

struct VS_INPUT
{
	// This is all of the stuff that we ever use.
	float4 vPos             : POSITION;
	float4 vColor           : COLOR;
	float4 vNormal          : NORMAL;
	float3 vSpecular        : SPECULAR;

	float3 vBoneWeights     : BONEWEIGHTS;
	uint4 vBoneIndices      : BONEINDICES;

	// make these float2's and stick the [n n 0 1] in the dot math.
	float4 vTexCoord0       : TEXCOORD0;
	float4 vTexCoord1       : TEXCOORD1;
	float4 vTexCoord2       : TEXCOORD2;
	float4 vTexCoord3       : TEXCOORD3;

	float3 vTangentS        : TANGENT_S;
	float3 vTangentT        : TANGENT_T;
	float4 vUserData        : USERDATA;

	// Position and normal/tangent deltas
	float3 vPosFlex         : POSITION1;
	float3 vNormalFlex      : NORMAL1;
	int vVertexID           : SV_VertexID;
};

VS_OUTPUT main(const VS_INPUT v)
{
	VS_OUTPUT o;

	float4 vPosition = v.vPos;
	float3 vNormal = 0;
	float4 vTangent;
	if ((!VERTEXCOLOR && (DYNAMIC_LIGHT || STATIC_LIGHT_VERTEX)) ||
		FLASHLIGHT ||
		SEAMLESS_BASE ||
		SEAMLESS_DETAIL ||
		g_bDecalOffset ||
		CUBEMAP)
	{
		if (NORMALMAPPING)
			DecompressVertex_NormalTangent(v.vNormal, v.vUserData, vNormal, vTangent);
		else
			DecompressVertex_Normal(v.vNormal, vNormal);
	}

	float3 NNormal;
	if (SEAMLESS_BASE || SEAMLESS_DETAIL)
	{
		NNormal = normalize(vNormal);
		o.SeamlessWeights.xyz = NNormal * NNormal;
	}

	if (!MORPHING)
	{
		if (NORMALMAPPING)
			ApplyMorph(v.vPosFlex, v.vNormalFlex, vPosition.xyz, vNormal, vTangent.xyz);
		else
			ApplyMorph(v.vPosFlex, v.vNormalFlex, vPosition.xyz, vNormal);
	}
	else
	{
#if 0 // TODO
		if (NORMALMAPPING)
		{
			ApplyMorph(TEXINDEX_MORPH, SMPINDEX_MORPH, cMorphTargetTextureDim, cMorphSubrect,
				v.vVertexID, v.vTexCoord2.xyz, vPosition.xyz, vNormal, vTangent.xyz);
		}
		else
		{
			ApplyMorph(TEXINDEX_MORPH, SMPINDEX_MORPH, cMorphTargetTextureDim, cMorphSubrect,
				v.vVertexID, v.vTexCoord2.xyz, vPosition.xyz, vNormal);
		}
#endif
	}

	// Perform skinning
	float3 worldTangentS, worldTangentT;

	if (NORMALMAPPING)
	{
		SkinPositionNormalAndTangentSpace(g_bSkinning, vPosition, vNormal, vTangent,
			v.vBoneWeights, v.vBoneIndices, o.worldSpacePos,
			o.worldSpaceNormal, worldTangentS, worldTangentT);

		worldTangentS = normalize(worldTangentS);
		worldTangentT = normalize(worldTangentT);
	}
	else
	{
		o.boneWeightsOut = SkinPositionAndNormal(
			g_bSkinning,
			vPosition, vNormal,
			v.vBoneWeights, v.vBoneIndices,
			o.worldSpacePos, o.worldSpaceNormal);
	}

	if (NORMALMAPPING || !VERTEXCOLOR)
		o.worldSpaceNormal = normalize(o.worldSpaceNormal);

	if (MORPHING && DECAL)
		o.worldSpacePos += o.worldSpaceNormal * 0.05f * v.vTexCoord2.z;

	if (NORMALMAPPING)
	{
		o.worldSpaceTangent = float4(worldTangentS, vTangent.w);
		o.worldSpaceBinormal = worldTangentT;
	}

	// Transform into projection space
	o.vProjPos = mul(float4(o.worldSpacePos, 1), cViewProj);
	o.projPos = o.vProjPos;

	o.fogFactorW.w = CalcFog(o.worldSpacePos, o.vProjPos.xyz, DOWATERFOG);
	o.fog = o.fogFactorW.w;

	// Needed for cubemaps
	if (CUBEMAP)
		o.worldVertToEyeVector.xyz = cEyePos - o.worldSpacePos;

	if (VERTEXCOLOR)
	{
		// Assume that this is unlitgeneric if you are using vertex color.
		o.color.rgb = GAMMA_CONVERT_VERTEX_COLOR ? GammaToLinear(v.vColor.rgb) : v.vColor.rgb;
		o.color.a = v.vColor.a;
	}
	else
	{
		//o.color = float4(worldPos, 1);
		o.color = float4(DoLighting(o.worldSpacePos, o.worldSpaceNormal, v.vSpecular, STATIC_LIGHT_VERTEX, DYNAMIC_LIGHT, HALFLAMBERT), 1);
	}

	// TODO
	//if (SEAMLESS_BASE)
	//	o.baseTexCoord.xyz = SEAMLESS_SCALE * v.vPos.xyz;
	//else
	{
		// Base texture coordinates
		o.baseTexCoord.x = dot(v.vTexCoord0, cCustom.base.texCoordTransform[0]);
		o.baseTexCoord.y = dot(v.vTexCoord0, cCustom.base.texCoordTransform[1]);
	}

	// TODO
	//if (SEAMLESS_DETAIL)
	//{
		// FIXME: detail texcoord as a 2d xform doesn't make much sense here, so I just do enough so
		// that scale works. More smartness could allow 3d xform.
	//	o.detailTexCoord.xyz = (SEAMLESS_SCALE * cCustom.base.texCoordTransform[0].x) * v.vPos.xyz;
	//}
	//else
	{
		// Detail texture coordinates
		// FIXME: This shouldn't have to be computed all the time.
		o.detailTexCoord.x = dot(v.vTexCoord0, cCustom.base.texCoordTransform[0]);
		o.detailTexCoord.y = dot(v.vTexCoord0, cCustom.base.texCoordTransform[1]);
	}

	/*if (TEXACTIVE_LIGHTMAP0 && TEXACTIVE_LIGHTMAP1 && TEXACTIVE_LIGHTMAP2)
	{
		TODO: bumped lightmaps
	}
	else*/ if (TEXACTIVE_LIGHTMAP0)
	{
		o.lightmapTexCoord12.xy = v.vTexCoord1.xy;
	}

	// Light attenuation
	for (uint i = 0; i < NUM_LIGHTS; i++)
		o.lightAtten[i] = GetVertexAttenForLight(o.worldSpacePos, i);

	if (SEPARATE_DETAIL_UVS)
		o.detailTexCoord.xy = v.vTexCoord1.xy;

	return o;
}
