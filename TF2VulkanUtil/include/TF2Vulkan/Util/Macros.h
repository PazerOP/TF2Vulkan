#pragma once

#include "Threads.h"

#include <tier0/dbg.h>

#include <compare>
#include <string_view>

namespace Util
{
	void LogFunctionCall(const std::string_view& fnSig, const std::string_view& file, int line);
	[[noreturn]] void EnsureConditionFailed(const char* condition, const char* fnSig, const char* file, int line);
	void FunctionNotImplemented(const char* fnSig, const char* file, int line);
}

#define ASSERT_MAIN_THREAD() assert(::Util::IsMainThread())

#define TF2VULKAN_ENABLE_FUNCTION_LOGGING 1

#ifdef TF2VULKAN_ENABLE_FUNCTION_LOGGING
#define LOG_FUNC() { ASSERT_MAIN_THREAD(); ::Util::LogFunctionCall(__FUNCSIG__, __FILE__, __LINE__); }
#else
#define LOG_FUNC()
#endif

#define ENSURE(condition) { if (!(condition)) ::Util::EnsureConditionFailed(_T(#condition), __FUNCSIG__, __FILE__, __LINE__); }

#define TF2VULKAN_PREFIX "[TF2Vulkan] " __FUNCSIG__ ": "

#define NOT_IMPLEMENTED_FUNC_NOBREAK() ::Util::FunctionNotImplemented(__FUNCSIG__, __FILE__, __LINE__)

#define PRINTF_SV(stringView) Util::SafeConvert<int>((stringView).size()), (stringView).data()

#ifdef MATT_HAYNIE
#define NOT_IMPLEMENTED_FUNC() \
	{ NOT_IMPLEMENTED_FUNC_NOBREAK(); __debugbreak(); }
#else
#define NOT_IMPLEMENTED_FUNC() NOT_IMPLEMENTED_FUNC_NOBREAK()
#endif

#if defined(_MSC_VER) && defined(__INTELLISENSE__)
#define INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type) \
	bool operator==(const type&) const noexcept { __debugbreak(); return false; } \
	bool operator!=(const type&) const noexcept { __debugbreak(); return true; }

#define DEFAULT_STRONG_EQUALITY_OPERATOR(type) INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type)
#define DEFAULT_WEAK_EQUALITY_OPERATOR(type) INTELLISENSE_PLACEHOLDER_EQUALITY_OPERATORS(type)

#else
#define DEFAULT_STRONG_EQUALITY_OPERATOR(type) \
	std::strong_equality operator<=>(const type&) const noexcept = default;

#endif
