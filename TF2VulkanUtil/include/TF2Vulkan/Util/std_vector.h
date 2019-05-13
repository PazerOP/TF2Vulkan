#pragma once

#include "std_compare.h"

#include <vector>

#if false
#ifndef __INTELLISENSE__
template<typename T, typename Alloc1, typename Alloc2>
inline auto operator<=>(const std::vector<T, Alloc1>& lhs, const std::vector<T, Alloc2>& rhs)
{
	using RetType = std::common_comparison_category_t<Util::threeway_compare_result_t<size_t>, Util::threeway_compare_result_t<T>>;

	auto sizeResult = Util::strong_order(lhs.size(), rhs.size());
	if (!std::is_eq(sizeResult))
		return RetType(sizeResult);

	for (size_t i = 0; i < lhs.size(); i++)
	{
		const auto& lhsVal = lhs[i];
		const auto& rhsVal = rhs[i];

		auto result = lhsVal <=> rhsVal;
		if (!std::is_eq(result))
			return RetType(result);
	}

	return RetType(sizeResult); // Equality of some kind
}
#endif
#endif
