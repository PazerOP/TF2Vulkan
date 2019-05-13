#pragma once

#include "std_compare.h"

#include <variant>

#if false
#ifndef __INTELLISENSE__
template<typename... T>
inline auto operator<=>(const std::variant<T...>& lhs, const std::variant<T...>& rhs)
{
	using RetType = typename Util::common_comparison_category_t<std::strong_ordering, Util::threeway_compare_result_t<T>...>;
	auto indexResult = Util::strong_order(lhs.index(), rhs.index());
	if (!std::is_eq(indexResult))
		return RetType(indexResult);

	return RetType(std::visit([](const auto & lhsInner, const auto & rhsInner)
		{
			return lhsInner <=> rhsInner;

		}, lhs, rhs));
}
#endif
#endif
