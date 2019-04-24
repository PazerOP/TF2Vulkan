#include "TF2Vulkan/Util/interface.h"

#ifdef CUSTOM_CREATEINTERFACE_FN

InterfaceRegCustom* InterfaceRegCustom::s_pInterfaceRegs;

InterfaceRegCustom::InterfaceRegCustom(InstantiateInterfaceFn fn, const char* pName) :
	m_CreateFn(fn),
	m_pName(pName),
	m_pNext(s_pInterfaceRegs)
{
	s_pInterfaceRegs = this;
}
#endif
