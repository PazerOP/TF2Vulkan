#pragma once

#include <TF2Vulkan/Util/std_compare.h>

#include <cstdint>

#pragma pack(push, 1)

namespace TF2Vulkan{ namespace ShaderConstants
{
	struct alignas(4) bool32 final
	{
		constexpr bool32() = default;
		bool32(const bool32&) = default;
		bool32(bool32&&) = default;
		bool32& operator=(const bool32&) = default;
		bool32& operator=(bool32&&) = default;

		DEFAULT_STRONG_ORDERING_OPERATOR(bool32);

		operator bool() const { return !!m_Data; }
		bool32& operator=(bool v) { m_Data = v ? 1 : 0; return *this; }

		uint32_t m_Data = 0;
	};

	template<typename T, int size> struct vector;
	template<typename T, int sizeX, int sizeY> struct matrix;

	template<typename T>
	struct alignas(sizeof(T)) vector<T, 1>
	{
		constexpr vector() = default;
		using ThisType = vector<T, 1>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		T x = {};
	};
	template<typename T>
	struct alignas(sizeof(T) * 2) vector<T, 2>
	{
		constexpr vector() = default;
		using ThisType = vector<T, 2>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		T x = {};
		T y = {};
	};
	template<typename T>
	struct vector<T, 3>
	{
		constexpr vector() = default;
		using ThisType = vector<T, 3>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		T x = {};
		T y = {};
		T z = {};
	};
	template<typename T>
	struct alignas(sizeof(T) * 4) vector<T, 4>
	{
		constexpr vector() = default;
		using ThisType = vector<T, 4>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		T x = {};
		T y = {};
		T z = {};
		T w = {};
	};

	template<typename T, int sizeX>
	struct matrix<T, sizeX, 1>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeX, 1>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		vector<T, sizeX> x;
	};
	template<typename T, int sizeX>
	struct matrix<T, sizeX, 2>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeX, 2>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		vector<T, sizeX> x;
		vector<T, sizeX> y;
	};
	template<typename T, int sizeX>
	struct matrix<T, sizeX, 3>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeX, 3>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		vector<T, sizeX> x;
		vector<T, sizeX> y;
		vector<T, sizeX> z;
	};
	template<typename T, int sizeX>
	struct matrix<T, sizeX, 4>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeX, 4>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);

		vector<T, sizeX> x;
		vector<T, sizeX> y;
		vector<T, sizeX> z;
		vector<T, sizeX> w;
	};

	using bool1 = vector<bool32, 1>;
	using bool2 = vector<bool32, 2>;
	using bool3 = vector<bool32, 3>;
	using bool4 = vector<bool32, 4>;

	using int1 = vector<int32_t, 1>;
	using int2 = vector<int32_t, 2>;
	using int3 = vector<int32_t, 3>;
	using int4 = vector<int32_t, 4>;

	using uint1 = vector<uint32_t, 1>;
	using uint2 = vector<uint32_t, 2>;
	using uint3 = vector<uint32_t, 3>;
	using uint4 = vector<uint32_t, 4>;

	using float1 = vector<float, 1>;
	using float2 = vector<float, 2>;
	using float3 = vector<float, 3>;
	using float4 = vector<float, 4>;

	static_assert(sizeof(float1) == sizeof(float) * 1);
	static_assert(sizeof(float2) == sizeof(float) * 2);
	static_assert(sizeof(float3) == sizeof(float) * 3);
	static_assert(sizeof(float4) == sizeof(float) * 4);

	static_assert(alignof(float2) == 8);
	static_assert(alignof(float4) == 16);

	template<typename T> using matrix4x1 = matrix<T, 4, 1>;
	template<typename T> using matrix4x2 = matrix<T, 4, 2>;
	template<typename T> using matrix4x3 = matrix<T, 4, 3>;
	template<typename T> using matrix4x4 = matrix<T, 4, 4>;

	using float4x1 = matrix4x1<float>;
	using float4x2 = matrix4x2<float>;
	using float4x3 = matrix4x3<float>;
	using float4x4 = matrix4x4<float>;
} }

#pragma pack(pop)
