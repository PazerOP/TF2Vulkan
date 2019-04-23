#pragma once

#include <string>
#include <string_view>

namespace Util
{
	class PooledString final
	{
	public:
		explicit PooledString(const char* str);
		explicit PooledString(const std::string& str);

		const char* c_str() const { return m_Str; }

		operator const char* () const { return c_str(); }

		bool operator<(const char* str) const;
		bool operator<(const PooledString& str) const;

	private:
		const char* m_Str = nullptr;
	};
}

namespace std
{
	template<> struct hash<Util::PooledString>
	{
		size_t operator()(const Util::PooledString& ps) const;
	};
}
