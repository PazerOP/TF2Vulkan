#pragma once

#include <cstdint>
#include <type_traits>

namespace Util
{
	union FourCC final
	{
		FourCC() = default;
		constexpr FourCC(uint32_t i) : m_Int(i) {}
		constexpr FourCC(char c1, char c2, char c3, char c4) :
			m_Chars{ c1, c2, c3, c4 }
		{
		}
		template<size_t s, typename = std::enable_if_t<(s == 5)>>
		constexpr FourCC(const char(&buf)[s]) :
			m_Chars{ buf[0], buf[1], buf[2], buf[3] }
		{
		}

		constexpr bool operator==(const FourCC& other) const
		{
			return m_Int == other.m_Int;
		}
		constexpr bool operator!=(const FourCC& other) const
		{
			return !operator==(other);
		}

		uint32_t m_Int;
		char m_Chars[4];
	};
	static_assert(sizeof(FourCC) == 4);
}
