#ifndef INCLUDE_GUARD_SHADER_SHARED_H
#define INCLUDE_GUARD_SHADER_SHARED_H

#ifdef __cplusplus
#include <TF2Vulkan/AlignedTypes.h>
#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan{ namespace Shaders
{
#define FLOAT3_ARRAY(name, size) float3_aligned name[size];
#else
#define FLOAT3_ARRAY(name, size) float3 name[size]
#define constexpr const
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

#ifdef __cplusplus
		// Padding
		uint32_t : 32;
		uint32_t : 32;
#endif
	};

#ifdef __cplusplus
	CHECK_OFFSET(LightInfo, color, 0);
	CHECK_OFFSET(LightInfo, bIsDirectional, 12);
	CHECK_OFFSET(LightInfo, dir, 16);
	CHECK_OFFSET(LightInfo, bIsSpot, 28);
	CHECK_OFFSET(LightInfo, pos, 32);
	CHECK_OFFSET(LightInfo, falloff, 44);
	CHECK_OFFSET(LightInfo, atten, 48);
	CHECK_OFFSET(LightInfo, stopdot1, 60);
	CHECK_OFFSET(LightInfo, stopdot2, 64);
	CHECK_OFFSET(LightInfo, OOdot, 68);

	CHECK_SIZE(LightInfo, 80);
#endif

#if false
	struct PixelShaderLightInfo
	{
#ifdef __cplusplus
		DEFAULT_WEAK_EQUALITY_OPERATOR(PixelShaderLightInfo);
#endif
		float4 color;
		float4 pos;
	};
#endif

	struct AmbientLightCube
	{
		FLOAT3_ARRAY(x, 2);
		FLOAT3_ARRAY(y, 2);
		FLOAT3_ARRAY(z, 2);
	};

#ifdef __cplusplus
	static_assert(offsetof(AmbientLightCube, x[0]) == 0);
	static_assert(offsetof(AmbientLightCube, x[1]) == 16);
	static_assert(offsetof(AmbientLightCube, y[0]) == 32);
	static_assert(offsetof(AmbientLightCube, y[1]) == 48);
	static_assert(offsetof(AmbientLightCube, z[0]) == 64);
	static_assert(offsetof(AmbientLightCube, z[1]) == 80);
	static_assert(sizeof(AmbientLightCube) == 96);
#endif

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
	static constexpr uint1 BINDING_CBUF_SHADERCOMMON = 0;
	static constexpr uint1 BINDING_CBUF_SHADERCUSTOM = 1;
	static constexpr uint1 BINDING_CBUF_VSMODELMATRICES = 2;
	static constexpr uint1 BINDING_TEX2D = 9;
	static constexpr uint1 BINDING_TEX3D = 10;
	static constexpr uint1 BINDING_SAMPLERS = 11;
#else
#define BINDING_CBUF_SHADERCOMMON 0
#define BINDING_CBUF_SHADERCUSTOM 1
#define BINDING_CBUF_VSMODELMATRICES 2
#define BINDING_TEX2D 9
#define BINDING_TEX3D 10
#define BINDING_SAMPLERS 11
#endif

	static constexpr int1 TEX_DEFAULT_COLOR_BLACK = -1;
	static constexpr int1 TEX_DEFAULT_COLOR_BLACK_TRANSPARENT = -2;
	static constexpr int1 TEX_DEFAULT_COLOR_WHITE = -3;
	static constexpr int1 TEX_DEFAULT_COLOR_WHITE_TRANSPARENT = -4;
	static constexpr int1 TEX_DEFAULT_COLOR_FLATNORMAL = -5;
	static constexpr int1 TEX_DEFAULT_COLOR_GREY50 = -6;

	static constexpr float4 TEX_DEFAULT_COLORS[] =
	{
		float4(0, 0, 0, 1),           // black (opaque)
		float4(0, 0, 0, 0),           // black (transparent)
		float4(1, 1, 1, 1),           // white (opaque)
		float4(1, 1, 1, 0),           // white (transparent)
		float4(0.5f, 0.5f, 1, 1),     // flat normal
		float4(0.5f, 0.5f, 0.5f, 1),  // 50% grey (opaque)
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
