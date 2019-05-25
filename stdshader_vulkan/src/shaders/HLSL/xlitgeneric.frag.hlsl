#include "common_ps_fxc.hlsli"

#define PIXEL_SHADER
#include "xlitgeneric.common.hlsli"

static void UpdateWorldNormalFromNormalMap(inout float3 worldSpaceNormal, float2 texCoord, float3 worldSpaceTangent, float3 worldSpaceBinormal)
{
	float3 normalTexel = BumpMapTexture.Sample(BumpMapTextureSampler, texCoord).xyz;
	float3 tangentSpaceNormal = normalTexel * 2.0f - 1.0f;

	worldSpaceNormal = Vec3TangentToWorldNormalized(tangentSpaceNormal, worldSpaceNormal, worldSpaceTangent, worldSpaceBinormal);
}

//-----------------------------------------------------------------------------
// Purpose: Compute scalar diffuse term with various optional tweaks such as
//          Half Lambert and ambient occlusion
//-----------------------------------------------------------------------------
static float3 DiffuseTerm(const float3 worldNormal, const float3 lightDir,
	const bool bDoAmbientOcclusion, const float fAmbientOcclusion)
{
	float fResult;

	float NDotL = dot(worldNormal, lightDir);				// Unsaturated dot (-1 to 1 range)

	if (HALFLAMBERT)
	{
		fResult = saturate(NDotL * 0.5 + 0.5);				// Scale and bias to 0 to 1 range

		if (TEXACTIVE_LIGHTWARP)
		{
			fResult *= fResult;								// Square
		}
	}
	else
	{
		fResult = saturate(NDotL);						// Saturate pure Lambertian term
	}

	if (bDoAmbientOcclusion)
	{
		// Raise to higher powers for darker AO values
//		float fAOPower = lerp( 4.0f, 1.0f, fAmbientOcclusion );
//		result *= pow( NDotL * 0.5 + 0.5, fAOPower );
		fResult *= fAmbientOcclusion;
	}

	float3 fOut = float3(fResult, fResult, fResult);
	if (TEXACTIVE_LIGHTWARP)
	{
		fOut = 2.0f * LightWarpTexture.Sample(LightWarpTextureSampler, fResult);
	}

	return fOut;
}

static float3 PixelShaderDoGeneralDiffuseLight(const float fAtten, const float3 worldPos, const float3 worldNormal,
	const float3 vPosition, const float3 vColor,
	const bool bDoAmbientOcclusion, const float fAmbientOcclusion)
{
	float3 lightDir = normalize(vPosition - worldPos);
	return vColor * fAtten * DiffuseTerm(worldNormal, lightDir, bDoAmbientOcclusion, fAmbientOcclusion);
}

// Better suited to Pixel shader models, 11 instructions in pixel shader
// ... actually, now only 9: mul, cmp, cmp, mul, mad, mad, mad, mad, mad
static float3 PixelShaderAmbientLight(const float3 worldNormal)
{
	float3 linearColor, nSquared = worldNormal * worldNormal;
	float3 isNegative = (worldNormal >= 0.0) ? 0 : nSquared;
	float3 isPositive = (worldNormal >= 0.0) ? nSquared : 0;
	linearColor = isPositive.x * cAmbientCube.x[0] + isNegative.x * cAmbientCube.x[1] +
		isPositive.y * cAmbientCube.y[0] + isNegative.y * cAmbientCube.y[1] +
		isPositive.z * cAmbientCube.z[0] + isNegative.z * cAmbientCube.z[1];
	return linearColor;
}

static void UnpackPSLightInfo(const uint index, out float3 pos, out float3 color)
{
	pos = cLightInfo[index].pos.xyz;
	color = cLightInfo[index].color.xyz;
}

static float3 PixelShaderDoLightingLinear(const float3 worldPos, const float3 worldNormal,
	const float3 staticLightingColor, const bool bStaticLight,
	const float4 lightAtten,
	const bool bDoAmbientOcclusion, const float fAmbientOcclusion)
{
	float3 linearColor = 0.0f;

	if (bStaticLight)
	{
		// The static lighting comes in in gamma space and has also been premultiplied by $cOOOverbright
		// need to get it into
		// linear space so that we can do adds.
		linearColor += GammaToLinear(staticLightingColor * cOverbright);
	}

	if (AMBIENT_LIGHT)
	{
		float3 ambient = PixelShaderAmbientLight(worldNormal);

		if (bDoAmbientOcclusion)
			ambient *= fAmbientOcclusion * fAmbientOcclusion;	// Note squaring...

		linearColor += ambient;
	}

	for (uint i = 0; i < NUM_LIGHTS; i++)
	{
		float3 lightPos, lightColor;
		UnpackPSLightInfo(i, lightPos, lightColor);
		linearColor += PixelShaderDoGeneralDiffuseLight(lightAtten.x, worldPos, worldNormal,
			lightPos, lightColor,
			bDoAmbientOcclusion, fAmbientOcclusion);
	}

	return linearColor;
}

// Called directly by newer shaders or through the following wrapper for older shaders
static float3 PixelShaderDoLighting(const float3 worldPos, const float3 worldNormal,
	const float3 staticLightingColor, const bool bStaticLight, const float4 lightAtten,

	// New optional/experimental parameters
	const bool bDoAmbientOcclusion, const float fAmbientOcclusion)
{
	float3 linearColor = PixelShaderDoLightingLinear(worldPos, worldNormal, staticLightingColor,
		bStaticLight, lightAtten,
		bDoAmbientOcclusion, fAmbientOcclusion);

	// go ahead and clamp to the linear space equivalent of overbright 2 so that we match
	// everything else.
//		linearColor = HuePreservingColorClamp( linearColor, pow( 2.0f, 2.2 ) );

	return linearColor;
}

float4 main(PS_INPUT i) : SV_Target
{
	float3 diffuseColor = (float3)1;
	if (DIFFUSELIGHTING || VERTEXCOLOR)
		diffuseColor = i.color.bgr; // FIXME REALLY SOON: Why is this bgr????

	float4 baseTextureColor = (float4)1;
	if (TEXACTIVE_BASETEXTURE)
		baseTextureColor = BaseTexture.Sample(BaseTextureSampler, i.baseTexCoord.xy);

	if (NORMALMAPPING)
		UpdateWorldNormalFromNormalMap(i.worldSpaceNormal, i.baseTexCoord.xy, i.worldSpaceTangent, i.worldSpaceBinormal);

	if (DIFFUSELIGHTING)
	{
		diffuseColor = PixelShaderDoLighting(i.worldPos_ProjPosZ.xyz, i.worldSpaceNormal,
			float3(0.0f, 0.0f, 0.0f), false, i.lightAtten,
			false, 1.0f);
	}

	const float3 finalColor = LinearToGamma(diffuseColor.rgb) * baseTextureColor.rgb;
	const float finalAlpha = lerp(baseTextureColor.a, baseTextureColor.a * i.color.a, g_fVertexAlpha);
	return float4(finalColor, finalAlpha);
}
