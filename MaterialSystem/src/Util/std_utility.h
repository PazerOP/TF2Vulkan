#pragma once

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
}
