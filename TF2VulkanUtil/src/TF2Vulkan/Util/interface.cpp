#include "TF2Vulkan/Util/interface.h"

#undef _PREFAST_
#undef RTL_NUMBER_OF_V2

#include <Windows.h>

CSysModule* Util::FindModule(const char* moduleName)
{
	if (!moduleName || !moduleName[0])
	{
		assert(false);
		return nullptr;
	}

	return (CSysModule*)GetModuleHandleA(moduleName);
}

#ifdef CUSTOM_CREATEINTERFACE_FN

InterfaceRegCustom* InterfaceRegCustom::s_pInterfaceRegs;

InterfaceRegCustom::InterfaceRegCustom(InstantiateInterfaceFn fn, const char* pName) :
	m_CreateFn(fn),
	m_pName(pName),
	m_pNext(s_pInterfaceRegs)
{
	s_pInterfaceRegs = this;
}

CreateInterfaceFn TF2Vulkan::Sys_GetFactoryCustom(const char* moduleName)
{
	if (!moduleName || !moduleName[0])
		return nullptr;

	return Sys_GetFactoryCustom(FindModule(moduleName));
}

CreateInterfaceFn TF2Vulkan::Sys_GetFactoryCustom(CSysModule* pModule)
{
	if (!pModule)
		return nullptr;

	return reinterpret_cast<CreateInterfaceFn>(GetProcAddress((HMODULE)pModule, CREATEINTERFACE_PROCNAME));
}

#endif

void* Util::FindProcAddressRaw(CSysModule* module, const char* symbolName)
{
	if (!module)
		return nullptr;

	return GetProcAddress((HMODULE)module, symbolName);
}
