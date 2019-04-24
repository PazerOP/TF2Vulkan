#pragma once

#include <tier0/dbg.h>
#include <tier1/interface.h>

#ifdef CUSTOM_CREATEINTERFACE_FN

class InterfaceRegCustom
{
public:
	InterfaceRegCustom(InstantiateInterfaceFn fn, const char* pName);

public:
	InstantiateInterfaceFn	m_CreateFn;
	const char* m_pName;

	InterfaceRegCustom* m_pNext; // For the global list.
	static InterfaceRegCustom* s_pInterfaceRegs;
};

using InterfaceReg = InterfaceRegCustom;

DLL_EXPORT inline void* CreateInterface(const char* pName, int* pReturnCode)
{
	for (auto it = InterfaceReg::s_pInterfaceRegs; it; it = it->m_pNext)
	{
		if (!strcmp(pName, it->m_pName))
		{
			auto retVal = it->m_CreateFn();

			if (pReturnCode)
				* pReturnCode = retVal ? IFACE_OK : IFACE_FAILED;

			return retVal;
		}
	}

	Msg("[TF2Vulkan] Reporting %s as unavailable\n", pName);

	if (pReturnCode)
		* pReturnCode = IFACE_FAILED;

	return nullptr;
}

inline CreateInterfaceFn Sys_GetFactoryThis()
{
	return &CreateInterface;
}

#endif
