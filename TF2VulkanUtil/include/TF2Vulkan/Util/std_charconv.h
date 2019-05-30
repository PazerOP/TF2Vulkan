#pragma once

#include <charconv>
#include <string_view>
#include <optional>

namespace Util{ namespace charconv
{
	using std::from_chars;

	struct from_chars_result : public std::from_chars_result
	{
		from_chars_result(const std::from_chars_result& base, const std::string_view& initial) :
			std::from_chars_result(base)
		{
			if (ec == std::errc{})
				m_Remaining = initial.substr(base.ptr - initial.data());
			else
				m_Remaining = initial;
		}

		constexpr bool success() const { return ec == std::errc{}; }
		operator bool() const { return success(); }

		std::string_view m_Remaining;
	};

	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	from_chars_result from_chars(const std::string_view& sv, T& value,
		std::chars_format fmt = std::chars_format::general)
	{
		return from_chars_result(from_chars(sv.data(), sv.data() + sv.size(), value, fmt), sv);
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	from_chars_result from_chars(const std::string_view& sv, T& value,
		int base = 10)
	{
		return from_chars_result(from_chars(sv.data(), sv.data() + sv.size(), value, base), sv);
	}

	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	std::optional<T> from_chars_v(const std::string_view & sv, std::chars_format fmt = std::chars_format::general)
	{
		T temp{};
		if (from_chars(sv, temp, fmt))
			return temp;

		return std::nullopt;
	}

	template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	std::optional<T> from_chars_v(const std::string_view & sv, int base = 10)
	{
		T temp{};
		if (from_chars(sv, temp, base))
			return temp;

		return std::nullopt;
	}
} }
