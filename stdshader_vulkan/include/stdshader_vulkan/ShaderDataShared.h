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
		float3 color;
		float1 bIsDirectional;
		float3 dir;
		float1 bIsSpot;
		float3 pos;
		float1 falloff;
		float3 atten;
		float1 stopdot1;
		float1 stopdot2;
		float1 OOdot;
	};

#ifdef __cplusplus
	static_assert(offsetof(LightInfo, color) == 0);           // 0
	static_assert(offsetof(LightInfo, bIsDirectional) == 12); // 1
	static_assert(offsetof(LightInfo, dir) == 16);            // 2
	static_assert(offsetof(LightInfo, bIsSpot) == 28);        // 3
	static_assert(offsetof(LightInfo, pos) == 32);            // 4
	static_assert(offsetof(LightInfo, falloff) == 44);        // 5
	static_assert(offsetof(LightInfo, atten) == 48);          // 6
	static_assert(offsetof(LightInfo, stopdot1) == 60);       // 7
	static_assert(offsetof(LightInfo, stopdot2) == 64);       // 8
	static_assert(offsetof(LightInfo, OOdot) == 68);          // 9
#endif

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
