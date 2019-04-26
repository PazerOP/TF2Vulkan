#pragma once

#include <initializer_list>
#include <utility>

namespace Util
{
	template<typename T>
	constexpr __forceinline std::add_const_t<T>& as_const(T& t) noexcept
	{
		return t;
	}
	template<typename T>
	constexpr __forceinline std::add_const_t<T>* as_const(T* t) noexcept
	{
		return t;
	}

	template<typename T>
	[[nodiscard]] inline size_t hash_value(const T& value)
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

	[[nodiscard]] inline size_t hash_combine(const std::initializer_list<size_t>& hashes)
	{
		return ::Util::hash_combine_range(hashes.begin(), hashes.end());
	}

	template<typename... T>
	[[nodiscard]] inline size_t hash_multi(const T&... args)
	{
		return ::Util::hash_combine({ ::Util::hash_value(args)... });
	}

	template<typename TIter>
	[[nodiscard]] inline size_t hash_range(TIter begin, const TIter& end)
	{
		size_t retVal = 0;

		for (; begin != end; ++begin)
			retVal = hash_combine(retVal, hash_value(*begin));

		return retVal;
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
