#include "Util/utlsymbol.h"

size_t std::hash<CUtlSymbol>::operator()(const CUtlSymbol& s) const
{
	return std::hash<UtlSymId_t>{}(s);
}
size_t std::hash<CUtlSymbolDbg>::operator()(const CUtlSymbolDbg& s) const
{
	return std::hash<CUtlSymbol>{}(s);
}
