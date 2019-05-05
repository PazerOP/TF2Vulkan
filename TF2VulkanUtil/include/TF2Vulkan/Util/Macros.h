#pragma once

#include "Threads.h"

#include <tier0/dbg.h>

#include <string_view>

#include <tier0/basetypes.h>

namespace Util
{
	struct LogFunctionCallScope
	{
		LogFunctionCallScope(const std::string_view& fnSig, const std::string_view& file, int line, const std::string_view& msg);
		~LogFunctionCallScope();
	};

	void LogFunctionCall(const std::string_view& fnSig, const std::string_view& file, int line, const std::string_view& msg);
	[[noreturn]] void EnsureConditionFailed(const char* condition, const char* fnSig, const char* file, int line);
	void FunctionNotImplemented(const char* fnSig, const char* file, int line);
}

#define ASSERT_MAIN_THREAD() assert(::Util::IsMainThread())

#define TF2VULKAN_ENABLE_FUNCTION_LOGGING 1
#ifdef TF2VULKAN_ENABLE_FUNCTION_LOGGING
#define LOG_FUNC_MSG(msg) ::Util::LogFunctionCallScope EXPAND_CONCAT(_TF2Vulkan_LogFunctionCallScope_, __LINE__) (__FUNCSIG__, __FILE__, __LINE__, msg)
#else
#define LOG_FUNC_MSG(msg) { ASSERT_MAIN_THREAD(); }
#endif

#define LOG_FUNC() LOG_FUNC_MSG(std::string_view{})

#define ENSURE(condition) { if (!(condition)) ::Util::EnsureConditionFailed(_T(#condition), __FUNCSIG__, __FILE__, __LINE__); }

#define TF2VULKAN_PREFIX "[TF2Vulkan] " __FUNCSIG__ ": "

#define NOT_IMPLEMENTED_FUNC_NOBREAK() ::Util::FunctionNotImplemented(__FUNCSIG__, __FILE__, __LINE__)

#define PRINTF_SV(stringView) Util::SafeConvert<int>((stringView).size()), (stringView).data()
static constexpr const char* PRINTF_BOOL(bool val) { return val ? "true" : "false"; }

#ifdef MATT_HAYNIE
#define NOT_IMPLEMENTED_FUNC() \
	{ NOT_IMPLEMENTED_FUNC_NOBREAK(); __debugbreak(); throw 0; }
#else
#define NOT_IMPLEMENTED_FUNC() NOT_IMPLEMENTED_FUNC_NOBREAK()
#endif

#define ORDERING_OP_LT_IMPL(varName) \
	if ((varName) < (other. ## varName)) \
		return true; \
	else if ((other. ## varName) < (varName)) \
		return false;
