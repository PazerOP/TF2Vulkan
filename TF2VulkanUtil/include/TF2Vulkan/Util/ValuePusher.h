#pragma once

namespace Util
{
	template<typename T>
	class ValuePusher final
	{
	public:
		ValuePusher(T& target, const T& newValue) :
			m_Target(target),
			m_OldValue(std::move(target))
		{
			target = newValue;
		}
		ValuePusher(T& target, T&& newValue) :
			m_Target(target),
			m_OldValue(std::move(target))
		{
			target = std::move(newValue);
		}
		~ValuePusher()
		{
			m_Target = std::move(m_OldValue);
		}

	private:
		T& m_Target;
		T m_OldValue;
	};
}
