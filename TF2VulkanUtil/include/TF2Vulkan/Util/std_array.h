#pragma once

#include "std_compare.h"
#include "std_utility.h"

#include <array>

namespace std
{
	template<typename T, size_t size> struct hash<std::array<T, size>>
	{
		size_t operator()(const std::array<T, size>& a) const
		{
			return ::Util::hash_range(a.begin(), a.end());
		}
	};
}

#ifdef __INTELLISENSE__

#else
template<typename T, size_t size>
inline auto operator<=>(const std::array<T, size>& lhs, const std::array<T, size>& rhs)
{
	for (size_t i = 0; i < size; i++)
	{
		auto result = lhs[i] <=> rhs[i];
		if (!std::is_eq(result))
			return result;
	}

	// FIXME: How to deal with the fact that some stuff returns std::_Strong_ordering on MSVC?
	return lhs[0] <=> rhs[0];
	//return Util::strongest_equal_v<Util::threeway_compare_result_t<T>>;
}
#endif
