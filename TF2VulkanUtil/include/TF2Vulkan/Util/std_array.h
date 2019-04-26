#pragma once

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
