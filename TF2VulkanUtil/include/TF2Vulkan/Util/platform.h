#pragma once

#include <tier0/platform.h>

template<typename... T>
inline void Plat_DebugString(const char* fmt, const T& ... args)
{
	char buf[512];
	sprintf_s(buf, fmt, args...);
	Plat_DebugString(buf);
}
