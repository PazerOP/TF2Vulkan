#pragma once

#include <iterator>

namespace Util{ namespace iterator
{
	template<typename TIter>
	struct IterRangeWrapper
	{
		constexpr IterRangeWrapper(TIter begin, TIter end) : m_Begin(begin), m_End(end) {}

		auto begin() { return m_Begin; }
		auto end() { return m_End; }
		auto begin() const { return m_Begin; }
		auto end() const { return m_End; }

		TIter m_Begin;
		TIter m_End;
	};
} }
