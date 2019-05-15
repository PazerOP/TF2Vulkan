#pragma once

#include <algorithm>

namespace Util{ namespace algorithm
{
	using std::find;

	template<typename TContainer, typename TValue>
	inline bool contains(const TContainer& container, const TValue& value)
	{
		return std::find(std::begin(container), std::end(container), value) != std::end(container);
	}

	template<typename TContainer, typename TFunc>
	inline bool contains_if(const TContainer& container, const TFunc& func)
	{
		return std::find_if(std::begin(container), std::end(container), func) != std::end(container);
	}

	template<typename TContainer, typename TValue>
	inline auto find(const TContainer& container, const TValue& value)
	{
		return std::find(std::begin(container), std::end(container), value);
	}

	template<typename TContainer, typename TValue, typename TFunc>
	inline bool find_and(const TContainer& container, const TValue& value, const TFunc& func)
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

	template<typename T, size_t size>
	inline void copy(const T(&src)[size], T(&dest)[size])
	{
		std::copy(std::begin(src), std::end(src), dest);
	}

	template<typename T, typename T2>
	inline void copy(const T& src, T2& dest)
	{
		std::copy(std::begin(src), std::end(src), dest);
	}
} }
