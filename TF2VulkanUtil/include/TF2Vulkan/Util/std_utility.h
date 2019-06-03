#pragma once

#include <initializer_list>
#include <utility>

namespace Util
{
	using std::as_const;
	template<typename T>
	constexpr __forceinline std::add_const_t<T>* as_const(T* t) noexcept
	{
		return t;
	}

	template<typename T>
	[[nodiscard]] __forceinline size_t hash_value(const T& value)
	{
		return std::hash<T>{}(value);
	}

	size_t hash_combine(size_t h1, size_t h2);

	template<typename TIter>
	[[nodiscard]] inline size_t hash_combine_range(TIter begin, const TIter& end)
	{
		size_t retVal = 0;

		for (; begin != end; ++begin)
			retVal = ::Util::hash_combine(retVal, *begin);

		return retVal;
	}

	[[nodiscard]] __forceinline size_t hash_combine(const std::initializer_list<size_t>& hashes)
	{
		return ::Util::hash_combine_range(hashes.begin(), hashes.end());
	}

	template<typename TIter>
	[[nodiscard]] inline size_t hash_range(TIter begin, const TIter& end)
	{
		size_t retVal = 0;

		for (; begin != end; ++begin)
			retVal = hash_combine(retVal, hash_value(*begin));

		return retVal;
	}

	template<typename T, size_t size>
	[[nodiscard]] __forceinline size_t hash_value(const T(&value)[size])
	{
		return ::Util::hash_range(std::begin(value), std::end(value));
	}

	template<typename TOnly>
	[[nodiscard]] __forceinline size_t hash_multi(const TOnly& only)
	{
		return ::Util::hash_value(only);
	}
	template<typename TFirst, typename... TExtra>
	[[nodiscard]] __forceinline size_t hash_multi(const TFirst& first, const TExtra& ... args)
	{
		return ::Util::hash_combine({ ::Util::hash_value(first), ::Util::hash_value(args)... });
	}

	template<typename T, size_t size>
	inline constexpr auto& at(T(&array)[size], size_t i)
	{
		if (i >= size)
			throw std::out_of_range("Attempted to access element beyond the end of a C-style array");

		return array[i];
	}
	template<typename T>
	inline constexpr auto at(T& container, size_t i) -> decltype(container.at(i))
	{
		return container.at(i);
	}
}

#define STD_HASH_DEFINITION(type, ...) \
template<> struct ::std::hash<type> \
{ \
	inline size_t operator()(const type& v) const \
	{ \
		return ::Util::hash_multi(__VA_ARGS__); \
	} \
};
