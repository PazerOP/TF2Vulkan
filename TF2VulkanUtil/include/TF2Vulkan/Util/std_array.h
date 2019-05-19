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
			if constexpr (std::has_unique_object_representations_v<T> && ((sizeof(T) * 2) == sizeof(T[2])))
				return ::Util::hash_value(std::string_view(reinterpret_cast<const char*>(a.data()), sizeof(a) * sizeof(char)));
			else
				return ::Util::hash_range(a.begin(), a.end());
		}
	};
}

namespace Util{ namespace array
{
	namespace detail
	{
		template<typename T, size_t S1, size_t S2, size_t... I1, size_t... I2>
		static constexpr std::array<T, S1 + S2> concat_impl(
			const std::array<T, S1>& a1, const std::array<T, S2>& a2,
			std::index_sequence<I1...>, std::index_sequence<I2...>)
		{
			return std::array<T, S1 + S2>{ a1[I1]..., a2[I2]... };
		}
	}

	template<typename T, size_t S1, size_t S2>
	static constexpr std::array<T, S1 + S2> concat(const std::array<T, S1>& a1, const std::array<T, S2>& a2)
	{
		return detail::concat_impl(a1, a2, std::make_index_sequence<S1>{}, std::make_index_sequence<S2>{});
	}
} }

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
