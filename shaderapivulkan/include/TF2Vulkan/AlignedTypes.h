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

	template<typename T, int sizeX, int sizeY> struct matrix;
	template<typename T, size_t size, size_t alignment> struct vector;

	namespace detail
	{
		static constexpr bool IsVector(const void*) { return false; }
		template<typename T, size_t size, size_t alignment> static constexpr bool IsVector(const vector<T, size, alignment>*) { return true; }
		template<typename T> static constexpr bool is_vector_v = IsVector((T*)nullptr);
		template<typename T> static constexpr bool is_vector_or_scalar_v = is_vector_v<T> || std::is_arithmetic_v<T>;

		template<typename TLhs, typename TRhs> static constexpr bool is_valid_vec_op =
			is_vector_or_scalar_v<TLhs> && is_vector_or_scalar_v<TRhs> && !(std::is_arithmetic_v<TLhs> && std::is_arithmetic_v<TRhs>);

		template<typename T, int sizeY, int sizeX> static constexpr auto GetMatrixSizeX(const matrix<T, sizeX, sizeY>&) { return sizeX; }
		template<typename T, int sizeY, int sizeX> static constexpr auto GetMatrixSizeY(const matrix<T, sizeX, sizeY>&) { return sizeY; }

		template<typename T, size_t elements, size_t alignment> struct vector_storage;

		template<typename T, size_t alignment> struct alignas(alignment) vector_storage<T, 1, alignment>
		{
			T x{};
		};
		template<typename T, size_t alignment> struct alignas(alignment) vector_storage<T, 2, alignment>
		{
			T x{};
			T y{};
		};
		template<typename T, size_t alignment> struct alignas(alignment) vector_storage<T, 3, alignment>
		{
			constexpr vector_storage()
			{
				// vec3s must be aligned to 16 bytes, but we can't use alignas because
				// then MSVC makes the whole thing take 16 bytes rather than 12. We
				// still need to be able to pack [float3, float1] into 16 bytes.
				assert(uintptr_t(this) % (sizeof(T) * 4) == 0);
			}
			constexpr vector_storage(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

			T x{};
			T y{};
			T z{};
		};
		template<typename T, size_t alignment> struct alignas(alignment) vector_storage<T, 4, alignment>
		{
			T x{};
			T y{};
			T z{};
			T w{};
		};

		template<typename T, size_t size> static constexpr size_t GetDefaultAlignment()
		{
			return sizeof(T) * (size == 3 ? 1 : size);
		}
	}

	template<typename T, size_t size, size_t alignment = detail::GetDefaultAlignment<T, size>()>
	struct vector : detail::vector_storage<T, size, alignment>
	{
	private:
		using BaseType = detail::vector_storage<T, size, alignment>;

		template<typename TArg, typename... TInit>
		static constexpr BaseType InitBaseType(const TArg& arg1, const TInit& ... args)
		{
			constexpr bool BROADCAST = sizeof...(TInit) == 0 && size > 1;
			constexpr bool DIRECT_INIT = (size - 1) == sizeof...(TInit);
			static_assert(BROADCAST || DIRECT_INIT);

			if constexpr (DIRECT_INIT)
				return BaseType{ T(arg1), T(args)... };
			else if (BROADCAST)
			{
				BaseType retVal;

				retVal.x = (T)arg1;

				if constexpr (size >= 2)
					retVal.y = (T)arg1;
				if constexpr (size >= 3)
					retVal.z = (T)arg1;
				if constexpr (size >= 4)
					retVal.w = (T)arg1;

				return retVal;
			}
		}

	public:
		static_assert(size >= 1 && size <= 4);

		using ThisType = vector<T, size, alignment>;
		constexpr vector() = default;
		constexpr vector(const ThisType&) noexcept = default;
		constexpr vector(ThisType&&) noexcept = default;
		constexpr ThisType& operator=(const ThisType&) noexcept = default;
		constexpr ThisType& operator=(ThisType&&) noexcept = default;

		template<typename TX, typename... TOthers, typename = std::enable_if_t<sizeof...(TOthers) == size - 1>>
		constexpr vector(const TX& x_, const TOthers&... others) : BaseType(InitBaseType(x_, others...)) {}
		template<typename TAll>
		explicit constexpr vector(const TAll& all) : BaseType(InitBaseType(all)) {}

		template<typename TOther>
		constexpr vector(const vector<TOther, size>& other) : vector()
		{
			BaseType::x = other.x;
			if constexpr (size >= 2)
				BaseType::y = other.y;
			if constexpr (size >= 3)
				BaseType::z = other.z;
			if constexpr (size >= 4)
				BaseType::w = other.w;
		}

		static constexpr size_t ELEM_COUNT = size;

		T& operator[](size_t i) { assert(i < ELEM_COUNT); return *(&(BaseType::x) + i); }
		const T& operator[](size_t i) const { assert(i < ELEM_COUNT); return *(&(BaseType::x) + i); }

		template<typename OtherT>
		constexpr ThisType& operator=(const vector<OtherT, size>& other) noexcept
		{
			BaseType::x = other.x;

			if constexpr (size >= 2)
				BaseType::y = other.y;
			if constexpr (size >= 3)
				BaseType::z = other.z;
			if constexpr (size >= 4)
				BaseType::w = other.w;

			return *this;
		}

		template<typename = std::enable_if_t<size == 1>>
		ThisType& operator=(const T& other)
		{
			BaseType::x = other;
			return *this;
		}

		void SetFrom(const T* src)
		{
			BaseType::x = src[0];

			if constexpr (size >= 2)
				BaseType::y = src[1];
			if constexpr (size >= 3)
				BaseType::z = src[2];
			if constexpr (size >= 4)
				BaseType::w = src[3];
		}

#ifndef __INTELLISENSE__
		std::partial_ordering operator<=>(const ThisType& other) const
		{
			for (size_t i = 0; i < size; i++)
			{
				auto result = (*this)[i] <=> other[i];
				if (!std::is_eq(result))
					return result;
			}

			return std::partial_ordering::equivalent;
		}
#endif

		template<typename = std::enable_if_t<size == 1>>
		ThisType& Set(const T& x_)
		{
			BaseType::x = x_;
			return *this;
		}
		template<typename = std::enable_if_t<size == 2>>
		ThisType& Set(const T& x_, const T& y_)
		{
			BaseType::x = x_;
			BaseType::y = y_;
			return *this;
		}
		template<typename = std::enable_if_t<size == 3>>
		ThisType& Set(const T& x_, const T& y_, const T& z_)
		{
			BaseType::x = x_;
			BaseType::y = y_;
			BaseType::z = z_;
			return *this;
		}
		template<typename = std::enable_if_t<size == 4>>
		ThisType& Set(const T& x_, const T& y_, const T& z_, const T& w_)
		{
			BaseType::x = x_;
			BaseType::y = y_;
			BaseType::z = z_;
			BaseType::w = w_;
			return *this;
		}

		template<typename = std::enable_if_t<size == 1>>
		operator T & () { return BaseType::x; }
		template<typename = std::enable_if_t<size == 1>>
		constexpr operator const T & () const { return BaseType::x; }

		template<typename = std::enable_if_t<(size == 1) && std::is_same_v<T, bool32>>>
		constexpr operator bool() const { return BaseType::x; }
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

	using float3_aligned = vector<float, 3, 16>;

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

template<typename T, size_t elements>
constexpr inline TF2Vulkan::Shaders::vector<T, elements> operator-(const TF2Vulkan::Shaders::vector<T, elements>& v)
{
	TF2Vulkan::Shaders::vector<T, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = -v[i];

	return retVal;
}

template<typename TElemLeft, typename TElemRight, size_t elements>
inline auto operator+(const TF2Vulkan::Shaders::vector<TElemLeft, elements>& lhs, const TF2Vulkan::Shaders::vector<TElemRight, elements>& rhs)
{
	TF2Vulkan::Shaders::vector<std::common_type_t<TElemLeft, TElemRight>, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs[i] + rhs[i];

	return lhs;
}
template<typename TElemLeft, typename TElemRight, size_t elements>
inline TF2Vulkan::Shaders::vector<TElemLeft, elements>& operator+=(
	TF2Vulkan::Shaders::vector<TElemLeft, elements>& lhs, const TF2Vulkan::Shaders::vector<TElemRight, elements>& rhs)
{
	for (size_t i = 0; i < elements; i++)
		lhs[i] += rhs[i];

	return lhs;
}

template<typename TElemLhs, typename TElemRhs, size_t elements>
inline auto operator*(
	const TF2Vulkan::Shaders::vector<TElemLhs, elements>& lhs, const TF2Vulkan::Shaders::vector<TElemRhs, elements>& rhs)
{
	TF2Vulkan::Shaders::vector<std::common_type_t<TElemLhs, TElemRhs>, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs[i] * rhs[i];

	return retVal;
}
template<typename TElem, size_t elements, typename TScalar, typename = std::enable_if_t<std::is_arithmetic_v<TScalar>>>
inline TF2Vulkan::Shaders::vector<TElem, elements> operator*(
	const TF2Vulkan::Shaders::vector<TElem, elements>& lhs, const TScalar& rhs)
{
	TF2Vulkan::Shaders::vector<TElem, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs[i] * rhs;

	return retVal;
}

template<typename TElem, size_t elements, typename TScalar, typename = std::enable_if_t<std::is_arithmetic_v<TScalar>>>
inline TF2Vulkan::Shaders::vector<TElem, elements> operator*(
	const TScalar& lhs, const TF2Vulkan::Shaders::vector<TElem, elements>& rhs)
{
	TF2Vulkan::Shaders::vector<TElem, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs * rhs[i];

	return retVal;
}

template<typename TElem, size_t elements, typename TScalar>
inline TF2Vulkan::Shaders::vector<TElem, elements>& operator*=(TF2Vulkan::Shaders::vector<TElem, elements>& lhs, const TScalar& scalar)
{
	for (size_t i = 0; i < elements; i++)
		lhs[i] *= scalar;

	return lhs;
}

template<typename TElem, size_t elements, typename TScalar, typename = std::enable_if_t<std::is_arithmetic_v<TScalar>>>
inline auto operator/(const TScalar& lhs, const TF2Vulkan::Shaders::vector<TElem, elements>& rhs)
{
	TF2Vulkan::Shaders::vector<std::common_type_t<TScalar, TElem>, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs / rhs[i];

	return retVal;
}

template<typename TElem, size_t elements, typename TScalar, typename = std::enable_if_t<std::is_arithmetic_v<TScalar>>>
inline auto operator/(const TF2Vulkan::Shaders::vector<TElem, elements>& lhs, const TScalar& rhs)
{
	TF2Vulkan::Shaders::vector<std::common_type_t<TScalar, TElem>, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs[i] / rhs;

	return retVal;
}

template<typename TLhs, typename TRhs, size_t elements>
inline auto operator-(const TF2Vulkan::Shaders::vector<TLhs, elements>& lhs, const TF2Vulkan::Shaders::vector<TRhs, elements>& rhs)
{
	TF2Vulkan::Shaders::vector<std::common_type_t<TLhs, TRhs>, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs[i] - rhs[i];

	return retVal;
}

template<typename TLhs, typename TRhs, size_t elements, typename = std::enable_if_t<std::is_arithmetic_v<TLhs>>>
inline auto operator-(const TLhs& lhs, const TF2Vulkan::Shaders::vector<TRhs, elements>& rhs)
{
	TF2Vulkan::Shaders::vector<std::common_type_t<TLhs, TRhs>, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lhs - rhs[i];

	return retVal;
}

#ifndef __INTELLISENSE__
template<typename T>
inline auto operator<=>(const T& lhs, const TF2Vulkan::Shaders::vector<T, 1> & rhs)
{
	return lhs <=> rhs.x;
}
template<typename T>
inline auto operator<=>(const TF2Vulkan::Shaders::vector<T, 1>& lhs, const T& rhs)
{
	return lhs.x <=> rhs;
}
#endif
template<typename T>
inline auto operator!=(const T& lhs, const TF2Vulkan::Shaders::vector<T, 1> & rhs)
{
	return lhs != rhs.x;
}
template<typename T>
inline auto operator!=(const TF2Vulkan::Shaders::vector<T, 1> & lhs, const T& rhs)
{
	return lhs.x != rhs;
}

namespace TF2Vulkan{ namespace Shaders{ namespace detail
{
	template<typename TLhs, typename TRhs, size_t elements, typename TFunc>
	inline auto CreateVecFromOp(const TF2Vulkan::Shaders::vector<TLhs, elements>& lhs, const TF2Vulkan::Shaders::vector<TRhs, elements>& rhs,
		const TFunc& func)
	{
		using RetType = decltype(func(lhs, rhs));

		if constexpr (elements == 1)
			return vector<RetType, elements>(func(lhs.x, rhs.x));
		else if constexpr (elements == 2)
			return vector<RetType, elements>(func(lhs.x, rhs.x), func(lhs.y, rhs.y));
		else if constexpr (elements == 3)
			return vector<RetType, elements>(func(lhs.x, rhs.x), func(lhs.y, rhs.y), func(lhs.z, rhs.z));
		else if constexpr (elements == 4)
			return vector<RetType, elements>(func(lhs.x, rhs.x), func(lhs.y, rhs.y), func(lhs.z, rhs.z), func(lhs.w, rhs.w));
		else
			static_assert(false, "Invalid component count");
	}
	template<typename TLhs, typename TScalar, size_t elements, typename TFunc, typename = std::enable_if_t<std::is_arithmetic_v<TScalar>>>
	inline auto CreateVecFromOp(const TF2Vulkan::Shaders::vector<TLhs, elements>& lhs, const TScalar& rhs,
		const TFunc& func)
	{
		using RetType = decltype(func(lhs, rhs));

		if constexpr (elements == 1)
			return vector<RetType, elements>(func(lhs.x, rhs));
		else if constexpr (elements == 2)
			return vector<RetType, elements>(func(lhs.x, rhs), func(lhs.y, rhs));
		else if constexpr (elements == 3)
			return vector<RetType, elements>(func(lhs.x, rhs), func(lhs.y, rhs), func(lhs.z, rhs));
		else if constexpr (elements == 4)
			return vector<RetType, elements>(func(lhs.x, rhs), func(lhs.y, rhs), func(lhs.z, rhs), func(lhs.w, rhs));
		else
			static_assert(false, "Invalid component count");
	}
	template<typename TScalar, typename TRhs, size_t elements, typename TFunc, typename = std::enable_if_t<std::is_arithmetic_v<TScalar>>>
	inline auto CreateVecFromOp(const TScalar& lhs, const TF2Vulkan::Shaders::vector<TRhs, elements>& rhs,
		const TFunc& func)
	{
		using RetType = decltype(func(lhs, rhs));

		if constexpr (elements == 1)
			return vector<RetType, elements>(func(lhs, rhs.x));
		else if constexpr (elements == 2)
			return vector<RetType, elements>(func(lhs, rhs.x), func(lhs, rhs.y));
		else if constexpr (elements == 3)
			return vector<RetType, elements>(func(lhs, rhs.x), func(lhs, rhs.y), func(lhs, rhs.z));
		else if constexpr (elements == 4)
			return vector<RetType, elements>(func(lhs, rhs.x), func(lhs, rhs.y), func(lhs, rhs.z), func(lhs, rhs.w));
		else
			static_assert(false, "Invalid component count");
	}
} } }

template<typename TLhs, typename TRhs, typename = std::enable_if_t<TF2Vulkan::Shaders::detail::is_valid_vec_op<TLhs, TRhs>>>
inline auto operator<(const TLhs& lhs, const TRhs& rhs)
{
	return TF2Vulkan::Shaders::detail::CreateVecFromOp(lhs, rhs,
		[](const auto& l, const auto& r) -> TF2Vulkan::Shaders::bool32 { return l < r; });
}

template<typename TLhs, typename TRhs, typename = std::enable_if_t<TF2Vulkan::Shaders::detail::is_valid_vec_op<TLhs, TRhs>>>
inline auto operator>(const TLhs& lhs, const TRhs& rhs)
{
	return TF2Vulkan::Shaders::detail::CreateVecFromOp(lhs, rhs,
		[](const auto& l, const auto& r) -> TF2Vulkan::Shaders::bool32 { return l > r; });
}
