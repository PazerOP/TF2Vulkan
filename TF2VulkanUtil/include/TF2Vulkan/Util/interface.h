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

			Msg("[TF2Vulkan] %s(): Found %s\n", __FUNCTION__, pName);
			return retVal;
		}
	}

	Msg("[TF2Vulkan] %s(): Reporting %s as unavailable\n", __FUNCTION__, pName);

	if (pReturnCode)
		* pReturnCode = IFACE_FAILED;

	return nullptr;
}

inline CreateInterfaceFn Sys_GetFactoryThis()
{
	return &CreateInterface;
}

namespace TF2Vulkan
{
	CreateInterfaceFn Sys_GetFactoryCustom(CSysModule* pModule);
	CreateInterfaceFn Sys_GetFactoryCustom(const char* moduleName);
}
inline CreateInterfaceFn Sys_GetFactory(const char* moduleName)
{
	return TF2Vulkan::Sys_GetFactoryCustom(moduleName);
}
inline CreateInterfaceFn Sys_GetFactory(CSysModule* pModule)
{
	return TF2Vulkan::Sys_GetFactoryCustom(pModule);
}

#endif

namespace Util
{
	template<typename T>
	inline void ConnectInterface(CreateInterfaceFn factory, const char* interfaceName, T*& interfacePtr)
	{
		if (!factory)
		{
			assert(factory);
			Error("[TF2Vulkan] " __FUNCTION__ "(): Factory was null");
		}

		if (!(interfacePtr = (T*)factory(interfaceName, nullptr)))
		{
			assert(interfacePtr);
			Error("[TF2Vulkan] " __FUNCTION__ "(): Failed to connect to %s", interfaceName);
		}
	}

	CSysModule* FindModule(const char* moduleName);

	void* FindProcAddressRaw(CSysModule* module, const char* symbolName);

	template<typename T>
	inline T FindProcAddress(const char* moduleName, const char* symbolName)
	{
		static_assert(std::is_function_v<std::remove_pointer_t<T>> && std::is_pointer_v<T>);
		return reinterpret_cast<T>(FindProcAddressRaw(FindModule(moduleName), symbolName));
	}
}
