#include "Util/PooledString.h"

#include <tier1/stringpool.h>

#include <mutex>

using namespace Util;

namespace
{
	struct StringPoolData
	{
		CStringPool m_Pool;
		std::mutex m_Mutex;
	};
}

static auto& GetStringPool()
{
	static StringPoolData s_Pool;
	return s_Pool;
}

static const char* FindOrCreateString(const char* src)
{
	auto& pool = GetStringPool();

	std::lock_guard lock(pool.m_Mutex);
	return pool.m_Pool.Allocate(src);
}

PooledString::PooledString(const char* str) :
	m_Str(FindOrCreateString(str))
{
}

PooledString::PooledString(const std::string& str) :
	m_Str(FindOrCreateString(str.c_str()))
{
}

bool PooledString::operator<(const char* str) const
{
	return strcmp(m_Str, str) < 0;
}

bool PooledString::operator<(const PooledString& str) const
{
	return m_Str < str.m_Str;
}

size_t std::hash<Util::PooledString>::operator()(const Util::PooledString& ps) const
{
	return std::hash<uintptr_t>{}(uintptr_t(ps.c_str()));
}
