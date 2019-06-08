#pragma once

#include <string_view>

#include "std_compare.h"

#ifndef __INTELLISENSE__
template<typename TChar, typename TTraits>
inline std::strong_ordering operator<=>(const std::basic_string_view<TChar, TTraits>& lhs, const std::basic_string_view<TChar, TTraits>& rhs)
{
	if (lhs < rhs)
		return std::strong_ordering::less;
	else if (rhs < lhs)
		return std::strong_ordering::greater;
	else
		return std::strong_ordering::equal;
}
#endif
