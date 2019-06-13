#pragma once

#include <cstddef>
#include <type_traits>

namespace Util
{
	template<typename T, typename TOffset, typename = std::enable_if_t<std::is_integral_v<TOffset>>>
	__forceinline T* OffsetPtr(T* base, TOffset offset)
	{
		if (!base)
			return base;

		using byteType = std::conditional_t<std::is_const_v<T>, const std::byte*, std::byte*>;
		return reinterpret_cast<T*>(reinterpret_cast<byteType>(base) + offset);
	}
}
