#ifndef INCLUDE_GUARD_COMMON_VS_FXC_HLSLI
#define INCLUDE_GUARD_COMMON_VS_FXC_HLSLI

#include "common_fxc.hlsli"

// We're testing 2 normal compression methods
// One compressed normals+tangents into a SHORT2 each (8 bytes total)
// The other compresses them together, into a single UBYTE4 (4 bytes total)
// FIXME: pick one or the other, compare lighting quality in important cases
#define COMPRESSED_NORMALS_SEPARATETANGENTS_SHORT2	0
#define COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4	1
//#define COMPRESSED_NORMALS_TYPE						COMPRESSED_NORMALS_SEPARATETANGENTS_SHORT2
#define COMPRESSED_NORMALS_TYPE					COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4

#define COMPRESSED_VERTS 1

#define FOGTYPE_RANGE				0
#define FOGTYPE_HEIGHT				1

[[vk::binding(BINDING_CBUF_VS_STANDARD)]] cbuffer VertexShaderStandardConstants
{
	float cOOGamma;
	float cOneThird;
	int g_nLightCountRegister;
	bool4 g_bLightEnabled;

	float3 cEyePos;
	float cWaterZ;

	float4 cFlexScale;

	float4x4 cModelViewProj;
	float4x4 cViewProj;

	float4 cModelViewProjZ;
	float4 cViewProjZ;

	FogParams cFogParams;

	LightInfo cLightInfo[4];
	AmbientLightCube cAmbientCube;

	float4x3 cModel[53];
};

static const float cOverbright = 2.0f;
static const float cOOOverbright = 1.0f / cOverbright;

#define g_nLightCount       g_nLightCountRegister.x

#define cFogEndOverFogRange cFogParams.endOverRange
#define cFogOne             cFogParams.one
#define cFogMaxDensity      cFogParams.maxDensity
#define cOOFogRange         cFogParams.OORange

void _DecompressShort2Tangent(float2 inputTangent, out float4 outputTangent)
{
	float2 ztSigns = sign(inputTangent);				// sign bits for z and tangent (+1 or -1)
	float2 xyAbs = abs(inputTangent);				// 1..32767
	outputTangent.xy = (xyAbs - 16384.0f) / 16384.0f;	// x and y
	outputTangent.z = ztSigns.x * sqrt(saturate(1.0f - dot(outputTangent.xy, outputTangent.xy)));
	outputTangent.w = ztSigns.y;
}

//-----------------------------------------------------------------------------------
// Same code as _DecompressShort2Tangent, just one returns a float4, one a float3
void _DecompressShort2Normal(float2 inputNormal, out float3 outputNormal)
{
	float4 result;
	_DecompressShort2Tangent(inputNormal, result);
	outputNormal = result.xyz;
}

//-----------------------------------------------------------------------------------
// Decompress normal+tangent together
void _DecompressShort2NormalTangent(float2 inputNormal, float2 inputTangent, out float3 outputNormal, out float4 outputTangent)
{
	// FIXME: if we end up sticking with the SHORT2 format, pack the normal and tangent into a single SHORT4 element
	//        (that would make unpacking normal+tangent here together much cheaper than the sum of their parts)
	_DecompressShort2Normal(inputNormal, outputNormal);
	_DecompressShort2Tangent(inputTangent, outputTangent);
}

//-----------------------------------------------------------------------------------
// Decompress just a normal from four-component compressed format (same as above)
// We expect this data to come from an unsigned UBYTE4 stream in the range of 0..255
// [ When compiled, this works out to approximately 17 asm instructions ]
void _DecompressUByte4Normal(float4 inputNormal,
	out float3 outputNormal)					// {nX, nY, nZ}
{
	float fOne = 1.0f;

	float2 ztSigns = (inputNormal.xy - 128.0f) < 0;				// sign bits for zs and binormal (1 or 0)  set-less-than (slt) asm instruction
	float2 xyAbs = abs(inputNormal.xy - 128.0f) - ztSigns;		// 0..127
	float2 xySigns = (xyAbs - 64.0f) < 0;						// sign bits for xs and ys (1 or 0)
	outputNormal.xy = (abs(xyAbs - 64.0f) - xySigns) / 63.0f;	// abs({nX, nY})

	outputNormal.z = 1.0f - outputNormal.x - outputNormal.y;		// Project onto x+y+z=1
	outputNormal.xyz = normalize(outputNormal.xyz);				// Normalize onto unit sphere

	outputNormal.xy *= lerp(fOne.xx, -fOne.xx, xySigns);			// Restore x and y signs
	outputNormal.z *= lerp(fOne.x, -fOne.x, ztSigns.x);			// Restore z sign
}

//=======================================================================================
// Decompress a normal and tangent from four-component compressed format
// We expect this data to come from an unsigned UBYTE4 stream in the range of 0..255
// The final vTangent.w contains the sign to use in the cross product when generating a binormal
void _DecompressUByte4NormalTangent(float4 inputNormal,
	out float3 outputNormal,   // {nX, nY, nZ}
	out float4 outputTangent)   // {tX, tY, tZ, sign of binormal}
{
	float fOne = 1.0f;

	float4 ztztSignBits = (inputNormal - 128.0f) < 0;						// sign bits for zs and binormal (1 or 0)  set-less-than (slt) asm instruction
	float4 xyxyAbs = abs(inputNormal - 128.0f) - ztztSignBits;		// 0..127
	float4 xyxySignBits = (xyxyAbs - 64.0f) < 0;							// sign bits for xs and ys (1 or 0)
	float4 normTan = (abs(xyxyAbs - 64.0f) - xyxySignBits) / 63.0f;	// abs({nX, nY, tX, tY})
	outputNormal.xy = normTan.xy;										// abs({nX, nY, __, __})
	outputTangent.xy = normTan.zw;										// abs({tX, tY, __, __})

	float4 xyxySigns = 1 - 2 * xyxySignBits;								// Convert sign bits to signs
	float4 ztztSigns = 1 - 2 * ztztSignBits;								// ( [1,0] -> [-1,+1] )

	outputNormal.z = 1.0f - outputNormal.x - outputNormal.y;			// Project onto x+y+z=1
	outputNormal.xyz = normalize(outputNormal.xyz);					// Normalize onto unit sphere
	outputNormal.xy *= xyxySigns.xy;										// Restore x and y signs
	outputNormal.z *= ztztSigns.x;										// Restore z sign

	outputTangent.z = 1.0f - outputTangent.x - outputTangent.y;			// Project onto x+y+z=1
	outputTangent.xyz = normalize(outputTangent.xyz);					// Normalize onto unit sphere
	outputTangent.xy *= xyxySigns.zw;										// Restore x and y signs
	outputTangent.z *= ztztSigns.z;										// Restore z sign
	outputTangent.w = ztztSigns.w;										// Binormal sign
}

void DecompressVertex_Normal(float4 inputNormal, out float3 outputNormal)
{
	if (COMPRESSED_VERTS == 1)
	{
		if (COMPRESSED_NORMALS_TYPE == COMPRESSED_NORMALS_SEPARATETANGENTS_SHORT2)
		{
			_DecompressShort2Normal(inputNormal.xy, outputNormal);
		}
		else // ( COMPRESSED_NORMALS_TYPE == COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4 )
		{
			_DecompressUByte4Normal(inputNormal, outputNormal);
		}
	}
	else
	{
		outputNormal = inputNormal.xyz;
	}
}

void DecompressVertex_NormalTangent(float4 inputNormal, float4 inputTangent, out float3 outputNormal, out float4 outputTangent)
{
	if (COMPRESSED_VERTS == 1)
	{
		if (COMPRESSED_NORMALS_TYPE == COMPRESSED_NORMALS_SEPARATETANGENTS_SHORT2)
		{
			_DecompressShort2NormalTangent(inputNormal.xy, inputTangent.xy, outputNormal, outputTangent);
		}
		else // ( COMPRESSED_NORMALS_TYPE == COMPRESSED_NORMALS_COMBINEDTANGENTS_UBYTE4 )
		{
			_DecompressUByte4NormalTangent(inputNormal, outputNormal, outputTangent);
		}
	}
	else
	{
		outputNormal = inputNormal.xyz;
		outputTangent = inputTangent;
	}
}

bool ApplyMorph(float3 vPosFlex, inout float3 vPosition)
{
	// Flexes coming in from a separate stream
	float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
	vPosition.xyz += vPosDelta;
	return true;
}

bool ApplyMorph(float3 vPosFlex, float3 vNormalFlex, inout float3 vPosition, inout float3 vNormal)
{
	// Flexes coming in from a separate stream
	float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
	float3 vNormalDelta = vNormalFlex.xyz * cFlexScale.x;
	vPosition.xyz += vPosDelta;
	vNormal += vNormalDelta;
	return true;
}

bool ApplyMorph(float3 vPosFlex, float3 vNormalFlex,
	inout float3 vPosition, inout float3 vNormal, inout float3 vTangent)
{
	// Flexes coming in from a separate stream
	float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
	float3 vNormalDelta = vNormalFlex.xyz * cFlexScale.x;
	vPosition.xyz += vPosDelta;
	vNormal += vNormalDelta;
	vTangent.xyz += vNormalDelta;
	return true;
}

bool ApplyMorph(float4 vPosFlex, float3 vNormalFlex,
	inout float3 vPosition, inout float3 vNormal, inout float3 vTangent, out float flWrinkle)
{
	// Flexes coming in from a separate stream
	float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
	float3 vNormalDelta = vNormalFlex.xyz * cFlexScale.x;
	flWrinkle = vPosFlex.w * cFlexScale.y;
	vPosition.xyz += vPosDelta;
	vNormal += vNormalDelta;
	vTangent.xyz += vNormalDelta;
	return true;
}

float4 SampleMorphDelta(Texture2D vt, SamplerState vtSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect, const float flVertexID, const float flField)
{
	float flColumn = floor(flVertexID / vMorphSubrect.w);

	float4 t;
	t.x = vMorphSubrect.x + vMorphTargetTextureDim.z * flColumn + flField + 0.5f;
	t.y = vMorphSubrect.y + flVertexID - flColumn * vMorphSubrect.w + 0.5f;
	t.xy /= vMorphTargetTextureDim.xy;
	t.z = t.w = 0.f;

	return vt.SampleLevel(vtSampler, t.xy, t.w);
}

// Optimized version which reads 2 deltas
void SampleMorphDelta2(Texture2D vt, SamplerState vtSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect, const float flVertexID, out float4 delta1, out float4 delta2)
{
	float flColumn = floor(flVertexID / vMorphSubrect.w);

	float4 t;
	t.x = vMorphSubrect.x + vMorphTargetTextureDim.z * flColumn + 0.5f;
	t.y = vMorphSubrect.y + flVertexID - flColumn * vMorphSubrect.w + 0.5f;
	t.xy /= vMorphTargetTextureDim.xy;
	t.z = t.w = 0.f;

	delta1 = vt.SampleLevel(vtSampler, t.xy, t.w);
	t.x += 1.0f / vMorphTargetTextureDim.x;
	delta2 = vt.SampleLevel(vtSampler, t.xy, t.w);
}

bool ApplyMorph(Texture2D morphTexture, SamplerState morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
	const float flVertexID, const float3 vMorphTexCoord,
	inout float3 vPosition)
{
	if (MORPHING)
	{
		if (!DECAL)
		{
			// Flexes coming in from a separate stream
			float4 vPosDelta = SampleMorphDelta(morphTexture, morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, 0);
			vPosition += vPosDelta.xyz;
		}
		else
		{
			float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
			float3 vPosDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w).xyz;
			vPosition += vPosDelta.xyz * vMorphTexCoord.z;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool ApplyMorph(Texture2D morphTexture, SamplerState morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
	const float flVertexID, const float3 vMorphTexCoord,
	inout float3 vPosition, inout float3 vNormal)
{
	if (MORPHING)
	{
		if (!DECAL)
		{
			float4 vPosDelta, vNormalDelta;
			SampleMorphDelta2(morphTexture, morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, vPosDelta, vNormalDelta);
			vPosition += vPosDelta.xyz;
			vNormal += vNormalDelta.xyz;
		}
		else
		{
			float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
			float3 vPosDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w).xyz;
			t.x += 1.0f / vMorphTargetTextureDim.x;
			float3 vNormalDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w).xyz;
			vPosition += vPosDelta.xyz * vMorphTexCoord.z;
			vNormal += vNormalDelta.xyz * vMorphTexCoord.z;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool ApplyMorph(Texture2D morphTexture, SamplerState morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
	const float flVertexID, const float3 vMorphTexCoord,
	inout float3 vPosition, inout float3 vNormal, inout float3 vTangent)
{
	if (MORPHING)
	{
		if (!DECAL)
		{
			float4 vPosDelta, vNormalDelta;
			SampleMorphDelta2(morphTexture, morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, vPosDelta, vNormalDelta);
			vPosition += vPosDelta.xyz;
			vNormal += vNormalDelta.xyz;
			vTangent += vNormalDelta.xyz;
		}
		else
		{
			float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
			float3 vPosDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w).xyz;
			t.x += 1.0f / vMorphTargetTextureDim.x;
			float3 vNormalDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w).xyz;
			vPosition += vPosDelta.xyz * vMorphTexCoord.z;
			vNormal += vNormalDelta.xyz * vMorphTexCoord.z;
			vTangent += vNormalDelta.xyz * vMorphTexCoord.z;
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool ApplyMorph(Texture2D morphTexture, SamplerState morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
	const float flVertexID, const float3 vMorphTexCoord,
	inout float3 vPosition, inout float3 vNormal, inout float3 vTangent, out float flWrinkle)
{
	if (MORPHING)
	{
		if (!DECAL)
		{
			float4 vPosDelta, vNormalDelta;
			SampleMorphDelta2(morphTexture, morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, vPosDelta, vNormalDelta);
			vPosition += vPosDelta.xyz;
			vNormal += vNormalDelta.xyz;
			vTangent += vNormalDelta.xyz;
			flWrinkle = vPosDelta.w;
		}
		else
		{
			float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
			float4 vPosDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w);
			t.x += 1.0f / vMorphTargetTextureDim.x;
			float3 vNormalDelta = morphTexture.SampleLevel(morphSampler, t.xy, t.w).xyz;

			vPosition += vPosDelta.xyz * vMorphTexCoord.z;
			vNormal += vNormalDelta.xyz * vMorphTexCoord.z;
			vTangent += vNormalDelta.xyz * vMorphTexCoord.z;
			flWrinkle = vPosDelta.w * vMorphTexCoord.z;
		}

		return true;
	}
	else
	{
		flWrinkle = 0.0f;
		return false;
	}
}

float4 DecompressBoneWeights(const float4 weights)
{
	float4 result = weights;

	if (COMPRESSED_VERTS)
	{
		// Decompress from SHORT2 to float. In our case, [-1, +32767] -> [0, +1]
		// NOTE: we add 1 here so we can divide by 32768 - which is exact (divide by 32767 is not).
		//       This avoids cracking between meshes with different numbers of bone weights.
		//       We use SHORT2 instead of SHORT2N for a similar reason - the GPU's conversion
		//       from [-32768,+32767] to [-1,+1] is imprecise in the same way.
		result += 1;
		result /= 32768;
	}

	return result;
}

void SkinPositionAndNormal(bool bSkinning, const float4 modelPos, const float3 modelNormal,
	const float4 boneWeights, float4 fBoneIndices,
	out float3 worldPos, out float3 worldNormal)
{
	// Needed for invariance issues caused by multipass rendering
	int3 boneIndices = D3DCOLORtoUBYTE4(fBoneIndices).xyz;

	if (!bSkinning)
	{
		worldPos = mul4x3(modelPos, cModel[0]);
		worldNormal = mul3x3(modelNormal, (float3x3)cModel[0]);
	}
	else // skinning - always three bones
	{
		float4x3 mat1 = cModel[boneIndices[0]];
		float4x3 mat2 = cModel[boneIndices[1]];
		float4x3 mat3 = cModel[boneIndices[2]];

		float3 weights = DecompressBoneWeights(boneWeights).xyz;
		weights[2] = 1 - (weights[0] + weights[1]);

		float4x3 blendMatrix = mat1 * weights[0] + mat2 * weights[1] + mat3 * weights[2];
		worldPos = mul4x3(modelPos, blendMatrix);
		worldNormal = mul3x3(modelNormal, (float3x3)blendMatrix);
	}
}

// Is it worth keeping SkinPosition and SkinPositionAndNormal around since the optimizer
// gets rid of anything that isn't used?
void SkinPositionNormalAndTangentSpace(
	bool bSkinning,
	const float4 modelPos, const float3 modelNormal,
	const float4 modelTangentS,
	const float4 boneWeights, float4 fBoneIndices,
	out float3 worldPos, out float3 worldNormal,
	out float3 worldTangentS, out float3 worldTangentT)
{
	int3 boneIndices = D3DCOLORtoUBYTE4(fBoneIndices);

	if (!bSkinning)
	{
		worldPos = mul4x3(modelPos, cModel[0]);
		worldNormal = mul3x3(modelNormal, (float3x3)cModel[0]);
		worldTangentS = mul3x3((float3)modelTangentS, (float3x3)cModel[0]);
	}
	else // skinning - always three bones
	{
		float4x3 mat1 = cModel[boneIndices[0]];
		float4x3 mat2 = cModel[boneIndices[1]];
		float4x3 mat3 = cModel[boneIndices[2]];

		float3 weights = DecompressBoneWeights(boneWeights).xyz;
		weights[2] = 1 - (weights[0] + weights[1]);

		float4x3 blendMatrix = mat1 * weights[0] + mat2 * weights[1] + mat3 * weights[2];
		worldPos = mul4x3(modelPos, blendMatrix);
		worldNormal = mul3x3(modelNormal, (float3x3)blendMatrix);
		worldTangentS = mul3x3((float3)modelTangentS, (float3x3)blendMatrix);
	}
	worldTangentT = cross(worldNormal, worldTangentS) * modelTangentS.w;
}

float RangeFog(const float3 projPos)
{
	return max(cFogMaxDensity, (-projPos.z * cOOFogRange + cFogEndOverFogRange));
}

float WaterFog(const float3 worldPos, const float3 projPos)
{
	float4 tmp;

	tmp.xy = float2(cWaterZ, cEyePos.z) - worldPos.z;

	// tmp.x is the distance from the water surface to the vert
	// tmp.y is the distance from the eye position to the vert

	// if $tmp.x < 0, then set it to 0
	// This is the equivalent of moving the vert to the water surface if it's above the water surface

	tmp.x = max(0.0f, tmp.x);

	// $tmp.w = $tmp.x / $tmp.y
	tmp.w = tmp.x / tmp.y;

	tmp.w *= projPos.z;

	// $tmp.w is now the distance that we see through water.

	return max(cFogMaxDensity, (-tmp.w * cOOFogRange + cFogOne));
}

float CalcFog(const float3 worldPos, const float3 projPos, const int fogType)
{
#if defined( _X360 )
	// 360 only does pixel fog
	return 1.0f;
#endif

	if (fogType == FOGTYPE_RANGE)
	{
		return RangeFog(projPos);
	}
	else
	{
#if SHADERMODEL_VS_2_0 == 1
		// We do this work in the pixel shader in dx9, so don't do any fog here.
		return 1.0f;
#else
		return WaterFog(worldPos, projPos);
#endif
	}
}

float CalcFog(const float3 worldPos, const float3 projPos, const bool bWaterFog)
{
	float flFog;
	if (!bWaterFog)
	{
		flFog = RangeFog(projPos);
	}
	else
	{
#if SHADERMODEL_VS_2_0 == 1
		// We do this work in the pixel shader in dx9, so don't do any fog here.
		flFog = 1.0f;
#else
		flFog = WaterFog(worldPos, projPos);
#endif
	}

	return flFog;
}

float3 AmbientLight(const float3 worldNormal)
{
	float3 nSquared = worldNormal * worldNormal;
	int3 isNegative = (worldNormal < 0.0);
	float3 linearColor;
	linearColor = nSquared.x * cAmbientCube.x[isNegative.x] +
		nSquared.y * cAmbientCube.y[isNegative.y] +
		nSquared.z * cAmbientCube.z[isNegative.z];
	return linearColor;
}

// The following "internal" routines are called "privately" by other routines in this file which
// handle the particular flavor of vs20 control flow appropriate to the original caller
float VertexAttenInternal(const float3 worldPos, int lightNum)
{
	float result = 0.0f;

	// Get light direction
	float3 lightDir = cLightInfo[lightNum].pos.xyz - worldPos;

	// Get light distance squared.
	float lightDistSquared = dot(lightDir, lightDir);

	// Get 1/lightDistance
	float ooLightDist = rsqrt(lightDistSquared);

	// Normalize light direction
	lightDir *= ooLightDist;

	float3 vDist = dst(lightDistSquared, ooLightDist).xyz;

	float flDistanceAtten = 1.0f / dot(cLightInfo[lightNum].atten.xyz, vDist);

	// Spot attenuation
	float flCosTheta = dot(cLightInfo[lightNum].dir.xyz, -lightDir);
	float flSpotAtten = (flCosTheta - cLightInfo[lightNum].spotParams.z) * cLightInfo[lightNum].spotParams.w;
	flSpotAtten = max(0.0001f, flSpotAtten);
	flSpotAtten = pow(flSpotAtten, cLightInfo[lightNum].spotParams.x);
	flSpotAtten = saturate(flSpotAtten);

	// Select between point and spot
	float flAtten = lerp(flDistanceAtten, flDistanceAtten * flSpotAtten, cLightInfo[lightNum].dir.w);

	// Select between above and directional (no attenuation)
	result = lerp(flAtten, 1.0f, cLightInfo[lightNum].color.w);

	return result;
}

float CosineTermInternal(const float3 worldPos, const float3 worldNormal, int lightNum, bool bHalfLambert)
{
	// Calculate light direction assuming this is a point or spot
	float3 lightDir = normalize(cLightInfo[lightNum].pos.xyz - worldPos);

	// Select the above direction or the one in the structure, based upon light type
	lightDir = lerp(lightDir, -cLightInfo[lightNum].dir.xyz, cLightInfo[lightNum].color.w);

	// compute N dot L
	float NDotL = dot(worldNormal, lightDir);

	if (!bHalfLambert)
	{
		NDotL = max(0.0f, NDotL);
	}
	else	// Half-Lambert
	{
		NDotL = NDotL * 0.5 + 0.5;
		NDotL = NDotL * NDotL;
	}
	return NDotL;
}

float3 DoLightInternal(const float3 worldPos, const float3 worldNormal, int lightNum, bool bHalfLambert)
{
	return (cLightInfo[lightNum].color *
		CosineTermInternal(worldPos, worldNormal, lightNum, bHalfLambert) *
		VertexAttenInternal(worldPos, lightNum)).xyz;
}

float3 DoLighting(const float3 worldPos, const float3 worldNormal,
	const float3 staticLightingColor, const bool bStaticLight,
	const bool bDynamicLight, bool bHalfLambert)
{
	float3 linearColor = float3(0.0f, 0.0f, 0.0f);

	if (bStaticLight)			// Static light
	{
		float3 col = staticLightingColor * cOverbright;
		linearColor += GammaToLinear(col);
	}

	if (bDynamicLight)			// Dynamic light
	{
		for (int i = 0; i < g_nLightCount; i++)
		{
			linearColor += DoLightInternal(worldPos, worldNormal, i, bHalfLambert);
		}
	}

	if (bDynamicLight)
	{
		linearColor += AmbientLight(worldNormal); //ambient light is already remapped
	}

	return linearColor;
}

#endif // INCLUDE_GUARD_COMMON_VS_FXC_HLSLI
