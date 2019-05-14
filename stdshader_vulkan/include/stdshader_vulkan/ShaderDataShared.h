#ifndef INCLUDE_GUARD_SHADER_SHARED_H
#define INCLUDE_GUARD_SHADER_SHARED_H

#ifdef __cplusplus
#include <TF2Vulkan/AlignedTypes.h>
#include <TF2Vulkan/Util/std_compare.h>
namespace TF2Vulkan{ namespace Shaders
{
#endif // __cplusplus

	struct LightInfo
	{
#ifdef __cplusplus
		DEFAULT_WEAK_EQUALITY_OPERATOR(LightInfo);
#endif
		float4 color;						// {xyz} is color	w is light type code (see comment below)
		float4 dir;							// {xyz} is dir		w is light type code
		float4 pos;
		float4 spotParams;
		float4 atten;
	};

	struct PixelShaderLightInfo
	{
#ifdef __cplusplus
		DEFAULT_WEAK_EQUALITY_OPERATOR(PixelShaderLightInfo);
#endif
		float4 color;
		float4 pos;
	};

	struct AmbientLightCube
	{
		float3 x[2];
		float3 y[2];
		float3 z[2];
	};

	struct FogParams
	{
#ifdef __cplusplus
		DEFAULT_WEAK_EQUALITY_OPERATOR(FogParams);
#endif
		float1 endOverRange;
		float1 one;
		float1 maxDensity;
		float1 OORange;
	};

#ifdef __cplusplus
} }

inline bool operator==(const TF2Vulkan::Shaders::AmbientLightCube& lhs,
	const TF2Vulkan::Shaders::AmbientLightCube& rhs)
{
	return
		lhs.x[0] == rhs.x[0] &&
		lhs.x[1] == rhs.x[1] &&
		lhs.y[0] == rhs.y[0] &&
		lhs.y[1] == rhs.y[1] &&
		lhs.z[0] == rhs.z[0] &&
		lhs.z[1] == rhs.z[1];
}

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const TF2Vulkan::Shaders::AmbientLightCube& lhs,
	const TF2Vulkan::Shaders::AmbientLightCube& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
#endif

#endif // __cplusplus
#endif // INCLUDE_GUARD_SHADER_SHARED_H
