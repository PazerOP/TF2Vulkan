#pragma once

#include "Threads.h"

#include <tier0/dbg.h>

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
	{ NOT_IMPLEMENTED_FUNC_NOBREAK(); __debugbreak(); throw 0; }
#else
#define NOT_IMPLEMENTED_FUNC() NOT_IMPLEMENTED_FUNC_NOBREAK()
#endif
