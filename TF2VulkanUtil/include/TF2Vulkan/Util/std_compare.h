#pragma once

#include <compare>

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
#define INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type) \
	bool operator==(const type&) const noexcept { __debugbreak(); return false; } \
	bool operator!=(const type&) const noexcept { __debugbreak(); return true; }

#define DEFAULT_STRONG_EQUALITY_OPERATOR(type) INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type)
#define DEFAULT_WEAK_EQUALITY_OPERATOR(type) INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type)

#else
#define DEFAULT_STRONG_EQUALITY_OPERATOR(type) \
	std::strong_equality operator<=>(const type&) const noexcept = default;

#define DEFAULT_WEAK_EQUALITY_OPERATOR(type) \
	std::weak_equality operator<=>(const type&) const noexcept = default;

#endif
