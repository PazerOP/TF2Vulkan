#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

namespace Util
{
	template<typename To, typename From>
	inline constexpr To SafeConvert(const From& f)
	{
		if constexpr (std::is_floating_point_v<To> || std::is_floating_point_v<From> ||
			std::is_same_v<To, From> || sizeof(To) > sizeof(From))
		{
			return To(f);
		}
		else
		{
			constexpr auto TO_MIN = intmax_t(std::numeric_limits<To>::min());
			constexpr auto TO_MAX = uintmax_t(std::numeric_limits<To>::max());
			constexpr auto FROM_MIN = intmax_t(std::numeric_limits<From>::min());
			constexpr auto FROM_MAX = uintmax_t(std::numeric_limits<From>::max());

			constexpr bool NEEDS_MIN_CHECK = TO_MIN > FROM_MIN;
			constexpr bool NEEDS_MAX_CHECK = TO_MAX < FROM_MAX;

			if constexpr (NEEDS_MIN_CHECK)
			{
				if (f < 0 && intmax_t(f) < TO_MIN)
				{
					assert(!"Truncating!");
					return To(TO_MIN);
				}
			}

			if constexpr (NEEDS_MAX_CHECK)
			{
				if (f > 0 && uintmax_t(f) > TO_MAX)
				{
					assert(!"Truncating!");
					return To(TO_MAX);
				}
			}

			return To(f);
		}
	}

	template<typename To, typename From>
	inline constexpr To SafeConvert(const From& f, To& t)
	{
		return t = SafeConvert<To, From>(f);
	}
}

#pragma pop_macro("min")
#pragma pop_macro("max")
