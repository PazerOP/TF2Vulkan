#pragma once

#include <algorithm>

namespace Util{ namespace algorithm
{
	using std::find;

	template<typename TContainer, typename TValue>
	inline bool contains(const TContainer& container, const TValue& value)
	{
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	template<typename TContainer, typename TValue>
	inline auto find(const TContainer& container, const TValue& value)
	{
		return std::find(std::begin(container), std::end(container), value);
	}

	template<typename TContainer, typename TValue, typename TFunc>
	inline auto find_and(const TContainer& container, const TValue& value, const TFunc& func) ->
		decltype(std::enable_if_t<std::is_same_v<void, std::invoke_result_t<TFunc, typename TContainer::const_iterator>>>{}, bool{})
	{
		if (auto found = find(container, value); found != std::end(container))
		{
			func(found);
			return true;
		}

		return false;
	}

	template<typename TContainer, typename TValue, typename TFunc>
	inline bool find_and(TContainer& container, const TValue& value, const TFunc& func)
	{
		if (auto found = find(container, value); found != std::end(container))
		{
			func(found);
			return true;
		}

		return false;
	}

	template<typename TContainer, typename TValue>
	inline bool try_erase(TContainer& container, const TValue& value)
	{
		return find_and<TContainer, TValue>(container, value, [&](auto found) { container.erase(found); });
	}
} }
