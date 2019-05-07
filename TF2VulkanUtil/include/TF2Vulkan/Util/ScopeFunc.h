#pragma once

namespace Util
{
	template<typename TEnterFunc, typename TExitFunc>
	class ScopeFunc final
	{
	public:
		ScopeFunc(TEnterFunc&& enterFunc = {}, TExitFunc&& exitFunc = {}) :
			m_EnterFunc(std::move(enterFunc)), m_ExitFunc(std::move(exitFunc))
		{
			m_EnterFunc();
		}
		~ScopeFunc()
		{
			m_ExitFunc();
		}

		ScopeFunc(const ScopeFunc&) = delete;
		ScopeFunc& operator=(const ScopeFunc&) = delete;
		ScopeFunc(ScopeFunc&&) = delete;
		ScopeFunc& operator=(ScopeFunc&&) = delete;

	private:
		[[no_unique_address]] TEnterFunc m_EnterFunc;
		[[no_unique_address]] TExitFunc m_ExitFunc;
	};
}
