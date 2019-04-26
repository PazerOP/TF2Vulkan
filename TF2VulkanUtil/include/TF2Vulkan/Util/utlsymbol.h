#pragma once

#include <tier1/utlsymbol.h>

namespace TF2Vulkan
{
	template<typename TBase>
	class CUtlSymbolDbgWrapper : public TBase
	{
	public:
		constexpr CUtlSymbolDbgWrapper() = default;

		template<typename... TArgs>
		CUtlSymbolDbgWrapper(TArgs&& ... args) : TBase(std::forward<TArgs>(args)...)
		{
#ifdef _DEBUG
			m_DbgString = TBase::String();
#endif
		}

	private:
#ifdef _DEBUG
		const char* m_DbgString = nullptr;
#endif
	};
}

using CUtlSymbolDbg = TF2Vulkan::CUtlSymbolDbgWrapper<CUtlSymbol>;

namespace std
{
	template<> struct hash<CUtlSymbol>
	{
		size_t operator()(const CUtlSymbol& s) const;
	};
	template<> struct hash<CUtlSymbolDbg>
	{
		size_t operator()(const CUtlSymbolDbg& s) const;
	};
}
