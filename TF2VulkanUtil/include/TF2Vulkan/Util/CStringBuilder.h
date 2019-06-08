#pragma once

#include <string_view>

namespace Util
{
	// Can't use stringstream due to linking issues
	template<size_t bufSize, typename TChar = char>
	class CStringBuilder
	{
	public:
		CStringBuilder(const std::basic_string_view<TChar>& initialValue = {})
		{
			Append(initialValue);
		}

		const TChar* c_str()
		{
			if (m_Length >= bufSize)
				throw std::overflow_error("Ran out of space when trying to add null terminator");

			m_Buffer[m_Length] = '\0';
			return m_Buffer;
		}

		std::basic_string_view<TChar> GetBuffer() const { return std::basic_string_view<TChar>(m_Buffer, m_Length); }

		void Append(const std::basic_string_view<TChar>& sv)
		{
			memcpy_s(&m_Buffer[m_Length], sizeof(TChar) * (bufSize - m_Length), sv.data(), sizeof(TChar) * sv.size());
			m_Length += sv.size();
		}

		template<typename... TArgs>
		__declspec(noinline) void AppendF(const char* fmt, const TArgs& ... args)
		{
			auto written = sprintf_s(&m_Buffer[m_Length], bufSize - m_Length, fmt, args...);
			if (written > 0)
				m_Length += written;
		}

	private:
		TChar m_Buffer[bufSize];
		size_t m_Length = 0;
	};
}
