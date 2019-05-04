#pragma once

#include <compare>
#include <stack>

#ifndef __INTELLISENSE__
template<typename T, typename TContainer>
std::common_comparison_category_t<std::weak_ordering, std::weak_equality>
operator<=>(const std::stack<T, TContainer>& lhs, const std::stack<T, TContainer>& rhs)
{
	if (auto result = lhs.size() <=> rhs.size(); !std::is_eq(result))
		return result;

	if (lhs < rhs)
		return std::weak_ordering::less;
	else if (lhs > rhs)
		return std::weak_ordering::greater;
	else
		return std::weak_equality::equivalent;
}
#endif

namespace Util
{

}
