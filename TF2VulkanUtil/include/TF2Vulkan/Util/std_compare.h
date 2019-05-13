#pragma once

#include "std_type_traits.h"
#include <compare>

namespace Util
{
	namespace detail
	{
		template<typename T>
		static constexpr auto ThreewayCompareResultImpl()
		{
			if constexpr (std::is_integral_v<T>)
				return std::strong_ordering::equal;
			else if constexpr (std::is_floating_point_v<T>)
				return std::weak_ordering::equivalent;
			else
				return std::declval<T>() <=> std::declval<T>();
		}
	}

	template<typename T> using threeway_compare_result_t = std::decay_t<decltype(detail::ThreewayCompareResultImpl<T>())>;

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

#if false
		template<typename... T> static constexpr auto CommonComparisonCategoryImpl()
		{
			static constexpr bool AllCompCategories =
				std::disjunction_v<Util::type_traits::is_any_of<T, std::weak_equality, std::strong_equality,
				std::partial_ordering, std::weak_ordering, std::strong_ordering>...>;
			static_assert(AllCompCategories);

			static constexpr bool AnyWeakEquality = Util::type_traits::is_any_of_v<std::weak_equality, T...>;
			static constexpr bool AnyStrongEquality = Util::type_traits::is_any_of_v<std::strong_equality, T...>;
			static constexpr bool AnyPartialOrdering = Util::type_traits::is_any_of_v<std::partial_ordering, T...>;
			static constexpr bool AnyWeakOrdering = Util::type_traits::is_any_of_v<std::weak_ordering, T...>;

			if constexpr (AnyWeakEquality || (AnyStrongEquality && (AnyPartialOrdering || AnyWeakOrdering)))
				return std::weak_equality::equivalent;
			else if constexpr (AnyStrongEquality)
				return std::strong_equality::equal;
			else if constexpr (AnyPartialOrdering)
			{
				static_assert(!AnyWeakEquality && !AnyStrongEquality);
				return std::partial_ordering::equivalent;
			}
			else if constexpr (AnyWeakOrdering)
				return std::weak_ordering::equivalent;
			else
			{
				static_assert(sizeof...(T) == 0 || std::conjunction_v<std::is_same_v<T, std::strong_equality>...>);
				return std::strong_equality::equal;
			}
		}
#endif
	}

	//template<typename... T> using common_comparison_category_t = decltype(detail::CommonComparisonCategoryImpl<T...>());

	template<typename T> static constexpr auto strongest_equal_v = detail::strongest_equal_impl<T>();
	template<typename T> static constexpr bool is_ordering_v = detail::is_ordering_impl<T>();

	template<typename T> static constexpr bool has_strong_ordering_v = std::is_same_v<threeway_compare_result_t<T>, std::strong_ordering>;
	template<typename T> static constexpr bool has_weak_ordering_v = std::is_same_v<threeway_compare_result_t<T>, std::weak_ordering>;
	template<typename T> static constexpr bool has_partial_ordering_v = std::is_same_v<threeway_compare_result_t<T>, std::partial_ordering>;
	template<typename T> static constexpr bool has_strong_equality_v = std::is_same_v<threeway_compare_result_t<T>, std::strong_equality>;
	template<typename T> static constexpr bool has_weak_equality_v = std::is_same_v<threeway_compare_result_t<T>, std::weak_equality>;

	template<typename T>
	constexpr std::strong_ordering strong_order(const T& lhs, const T& rhs)
	{
		if (lhs < rhs)
			return std::strong_ordering::less;
		else if (rhs < lhs)
			return std::strong_ordering::greater;
		else
			return std::strong_ordering::equal;
	}
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

