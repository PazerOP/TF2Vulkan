#ifndef SHADER_COMPONENT_DATA_HLSLI_INCLUDE_GUARD
#define SHADER_COMPONENT_DATA_HLSLI_INCLUDE_GUARD

struct BumpmapUniforms
{
	float4 texCoordTransform[2];
};

struct DetailUniforms
{
	float3 tint;
	float1 scale;
	float4 texCoordTransform[2];
	float1 blendFactor;
};

struct BaseUniforms
{
	float4 texCoordTransform[2];
};

struct MorphUniforms
{
	float4 subrect;
	float3 targetTextureDim;
};

struct SeamlessScaleUniforms
{
	float scale;
};

#endif // SHADER_COMPONENT_DATA_HLSLI_INCLUDE_GUARD
