#pragma once

#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/vmatrix.h>

#include <cstdint>

#pragma pack(push, 1)

namespace TF2Vulkan{ namespace Shaders
{
	struct alignas(4) bool32 final
	{
		constexpr bool32(bool value = false) : m_Data(value ? 1 : 0) {}
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

	namespace detail
	{
		template<typename T, int size> static constexpr auto GetVectorSize(const vector<T, size>&) { return size; }
		template<typename T, int sizeY, int sizeX> static constexpr auto GetMatrixSizeX(const matrix<T, sizeX, sizeY>&) { return sizeX; }
		template<typename T, int sizeY, int sizeX> static constexpr auto GetMatrixSizeY(const matrix<T, sizeX, sizeY>&) { return sizeY; }
	}

	template<typename T>
	struct alignas(sizeof(T)) vector<T, 1>
	{
		using ThisType = vector<T, 1>;
		constexpr vector() = default;
		vector(const ThisType&) = default;
		constexpr vector(const T& val) : x(val) {}
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 1;

		operator T& () { return x; }
		operator const T& () const { return x; }
		ThisType& operator=(const ThisType&) = default;
		ThisType& operator=(ThisType&&) noexcept = default;
		ThisType& operator=(const T& rhs) noexcept { x = rhs; return *this; }

		T x;
	};
	template<typename T>
	struct alignas(sizeof(T) * 2) vector<T, 2>
	{
		constexpr vector() = default;
		using ThisType = vector<T, 2>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 2;

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		T x;
		T y;
	};
	template<typename T>
	struct vector<T, 3>
	{
		constexpr vector() = default;
		using ThisType = vector<T, 3>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 3;

		void SetFrom(const T* src)
		{
			x = src[0];
			y = src[1];
			z = src[2];
		}

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		T x;
		T y;
		T z;
	};
	template<typename T>
	struct alignas(sizeof(T) * 4) vector<T, 4>
	{
		constexpr vector() = default;
		explicit constexpr vector(T all) : x(all), y(all), z(all), w(all) {}
		constexpr vector(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
		using ThisType = vector<T, 4>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 4;

		void SetFrom(const T* src)
		{
			x = src[0];
			y = src[1];
			z = src[2];
			w = src[3];
		}

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		ThisType& Set(const T& x_, const T& y_, const T& z_, const T& w_)
		{
			x = x_;
			y = y_;
			z = z_;
			w = w_;
			return *this;
		}

		T x;
		T y;
		T z;
		T w;
	};

	template<typename T, int sizeY>
	struct matrix<T, sizeY, 1>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeY, 1>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 1;

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		vector<T, sizeY> x;
	};
	template<typename T, int sizeY>
	struct matrix<T, sizeY, 2>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeY, 2>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 2;

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		vector<T, sizeY> x;
		vector<T, sizeY> y;
	};
	template<typename T, int sizeY>
	struct matrix<T, sizeY, 3>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeY, 3>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 3;

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		vector<T, sizeY> x;
		vector<T, sizeY> y;
		vector<T, sizeY> z;
	};
	template<typename T, int sizeY>
	struct matrix<T, sizeY, 4>
	{
		constexpr matrix() = default;
		using ThisType = matrix<T, sizeY, 4>;
		DEFAULT_PARTIAL_ORDERING_OPERATOR(ThisType);
		static constexpr size_t ELEM_COUNT = 4;

		auto& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&x + i); }
		auto& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&x + i); }

		void SetFrom(const VMatrix& mtx)
		{
			x.SetFrom(mtx.Base());
			y.SetFrom(mtx.Base() + 4);
			z.SetFrom(mtx.Base() + 8);
			w.SetFrom(mtx.Base() + 12);
		}

		vector<T, sizeY> x;
		vector<T, sizeY> y;
		vector<T, sizeY> z;
		vector<T, sizeY> w;
	};

	template<uint_fast8_t cols, uint_fast8_t rows, typename T = float>
	inline constexpr auto MatrixFromVMatrix(const VMatrix& vm)
	{
		matrix<T, cols, rows> retVal;

		for (uint_fast8_t y = 0; y < rows; y++)
		{
			for (uint_fast8_t x = 0; x < cols; x++)
				retVal[y][x] = vm.m[y][x];
		}

		return retVal;
	}

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
