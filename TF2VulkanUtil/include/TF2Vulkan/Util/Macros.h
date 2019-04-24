#pragma once

#include <tier0/dbg.h>

namespace Util
{
	void LogFunctionCall(const char* fnName, const char* fnSig, const char* file, int line);
	[[noreturn]] void EnsureConditionFailed(const char* condition, const char* fnSig, const char* file, int line);
}

#define LOG_FUNC() ::Util::LogFunctionCall(__FUNCTION__, __FUNCSIG__, __FILE__, __LINE__)

#define ENSURE(condition) { if (!(condition)) ::Util::EnsureConditionFailed(_T(#condition), __FUNCSIG__, __FILE__, __LINE__); }
