#pragma once

#include <type_traits>
#include <compare>

namespace Util
{
	template<typename T> using threeway_compare_result_t = std::decay_t<decltype(std::declval<T>() <=> std::declval<T>())>;

	namespace detail
	{
		template<typename T> constexpr auto strongest_equal_impl()
		{
			if constexpr (
				std::is_same_v<T, std::weak_equality> ||
				std::is_same_v<T, std::partial_ordering> ||
				std::is_same_v<T, std::weak_ordering>)
			{
				return T::equivalent;
			}
			else if constexpr (
				std::is_same_v<T, std::strong_ordering> ||
				std::is_same_v<T, std::strong_equality>)
			{
				return T::equal;
			}
			else
			{
				static_assert(false, "Invalid type");
			}
		}

		template<typename T> constexpr bool is_ordering_impl()
		{
			if constexpr (
				std::is_same_v<T, std::partial_ordering> ||
				std::is_same_v<T, std::weak_ordering> ||
				std::is_same_v<T, std::strong_ordering>)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		template<typename T> constexpr auto threeway_equal_impl()
		{
			if constexpr (std::is_same_v<T, std::weak_equality>)
				return std::weak_equality::equivalent;
			else if constexpr (std::is_same_v<T, std::strong_equality>)
				return std::strong_equality::equal;
		}
	}

	template<typename T> static constexpr auto strongest_equal_v = detail::strongest_equal_impl<T>();
	template<typename T> static constexpr bool is_ordering_v = detail::is_ordering_impl<T>();

	template<typename T> static constexpr bool has_strong_ordering_v = std::is_same_v<threeway_compare_result_t<T>, std::strong_ordering>;
	template<typename T> static constexpr bool has_weak_ordering_v = std::is_same_v<threeway_compare_result_t<T>, std::weak_ordering>;
	template<typename T> static constexpr bool has_partial_ordering_v = std::is_same_v<threeway_compare_result_t<T>, std::partial_ordering>;
	template<typename T> static constexpr bool has_strong_equality_v = std::is_same_v<threeway_compare_result_t<T>, std::strong_equality>;
	template<typename T> static constexpr bool has_weak_equality_v = std::is_same_v<threeway_compare_result_t<T>, std::weak_equality>;
}

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
#define INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type) \
	constexpr bool operator==(const type&) const noexcept { __debugbreak(); return false; } \
	constexpr bool operator!=(const type&) const noexcept { __debugbreak(); return false; }

#define INTELLISENSE_PLACEHOLDER_ORDERING_OPERATORS(type) \
	constexpr bool operator<(const type&) const noexcept { __debugbreak(); return false; } \
	constexpr bool operator<=(const type&) const noexcept { __debugbreak(); return false; } \
	constexpr bool operator>(const type&) const noexcept { __debugbreak(); return false; } \
	constexpr bool operator>=(const type&) const noexcept { __debugbreak(); return false; }

#define DEFAULT_STRONG_EQUALITY_OPERATOR(type) INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type)
#define DEFAULT_WEAK_EQUALITY_OPERATOR(type) INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type)

#define DEFAULT_WEAK_ORDERING_OPERATOR(type) \
	INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type) \
	INTELLISENSE_PLACEHOLDER_ORDERING_OPERATORS(type)

#define DEFAULT_STRONG_ORDERING_OPERATOR(type) \
	INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type) \
	INTELLISENSE_PLACEHOLDER_ORDERING_OPERATORS(type)

#define DEFAULT_PARTIAL_ORDERING_OPERATOR(type) \
	INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type) \
	INTELLISENSE_PLACEHOLDER_ORDERING_OPERATORS(type)

#else
#define DEFAULT_STRONG_EQUALITY_OPERATOR(type) \
	constexpr std::strong_equality operator<=>(const type&) const noexcept = default;

#define DEFAULT_WEAK_EQUALITY_OPERATOR(type) \
	constexpr std::weak_equality operator<=>(const type&) const noexcept = default;

#define DEFAULT_WEAK_ORDERING_OPERATOR(type) \
	constexpr std::weak_ordering operator<=>(const type&) const noexcept = default;

#define DEFAULT_STRONG_ORDERING_OPERATOR(type) \
	constexpr std::strong_ordering operator<=>(const type&) const noexcept = default;

#define DEFAULT_PARTIAL_ORDERING_OPERATOR(type) \
	constexpr std::partial_ordering operator<=>(const type&) const noexcept = default;

#endif

