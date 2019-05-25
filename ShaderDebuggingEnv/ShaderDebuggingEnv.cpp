// ShaderDebuggingEnv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

using namespace TF2Vulkan::Shaders;

namespace Env
{
	struct
	{
		float3 color = { 1, 1, 1 };
		float1 bIsDirectional = 1;
		float3 dir = { 0, 0, -1 };
		float1 bIsSpot = 0;
		float3 pos = { 0, 0, -2000000 };
		float1 falloff = 0;
		float3 atten = { 1, 0, 0 };
		float1 stopdot1 = 1;
		float1 stopdot2 = 1;
		float1 OOdot = 1;

	} static const cLightInfo[1];

	struct
	{
		float3 x[2];
		float3 y[2];
		float3 z[2];

	} static const cAmbientCube;

	static constexpr int g_nLightCount = std::size(cLightInfo);

	// The following "internal" routines are called "privately" by other routines in this file which
	// handle the particular flavor of vs20 control flow appropriate to the original caller
	float VertexAttenInternal(const float3 worldPos, int lightNum)
	{
		float result = 0.0f;

		// Get light direction
		float3 lightDir = cLightInfo[lightNum].pos - worldPos;

		// Get light distance squared.
		float lightDistSquared = dot(lightDir, lightDir);

		// Get 1/lightDistance
		float ooLightDist = rsqrt(lightDistSquared);

		// Normalize light direction
		lightDir *= ooLightDist;

		float3 vDist = (float3)dst(lightDistSquared, ooLightDist);

		float flDistanceAtten = 1.0f / dot(cLightInfo[lightNum].atten, vDist);

		// Spot attenuation
		float flCosTheta = dot(cLightInfo[lightNum].dir, -lightDir);
		float flSpotAtten = (flCosTheta - cLightInfo[lightNum].stopdot2) * cLightInfo[lightNum].OOdot;
		flSpotAtten = max(0.0001f, flSpotAtten);
		flSpotAtten = pow(flSpotAtten, cLightInfo[lightNum].falloff);
		flSpotAtten = saturate(flSpotAtten);

		// Select between point and spot
		float flAtten = lerp(flDistanceAtten, flDistanceAtten * flSpotAtten, cLightInfo[lightNum].bIsSpot);

		// Select between above and directional (no attenuation)
		result = lerp(flAtten, 1.0f, cLightInfo[lightNum].bIsDirectional);

		return result;
	}

	float CosineTermInternal(const float3 worldPos, const float3 worldNormal, int lightNum, bool bHalfLambert)
	{
		// Calculate light direction assuming this is a point or spot
		float3 lightDir = (float3)normalize(cLightInfo[lightNum].pos - worldPos);

		// Select the above direction or the one in the structure, based upon light type
		lightDir = lerp(lightDir, -cLightInfo[lightNum].dir, cLightInfo[lightNum].bIsDirectional);

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
			VertexAttenInternal(worldPos, lightNum));
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

	float3 DoLighting(const float3 worldPos, const float3 worldNormal,
		const float3 staticLightingColor, const bool bStaticLight,
		const bool bDynamicLight, bool bHalfLambert)
	{
		float3 linearColor = float3(0.0f, 0.0f, 0.0f);

#if false
		if (bStaticLight)			// Static light
		{
			float3 col = staticLightingColor * cOverbright;
			linearColor += GammaToLinear(col);
		}
#endif

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
}

int main()
{
	std::cout << "Hello World!\n";

	const float3 worldPos(-4.42067, -4.34696, 39.01047);
	const float3 worldNormal(-0.91896, 0.02635, 0.39346);
	auto result = Env::DoLighting(worldPos, worldNormal, {}, false, true, false);
}
