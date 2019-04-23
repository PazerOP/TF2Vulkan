
#include "TF2Vulkan/MaterialSystem.h"

#include <materialsystem/imaterialsystem.h>

extern "C" __declspec(dllexport) void* CreateInterface(const char* pName, int* pReturnCode)
{
	if (!strcmp(pName, MATERIAL_SYSTEM_INTERFACE_VERSION))
	{
		auto retVal = TF2Vulkan::GetMaterialSystem();
		if (pReturnCode)
			*pReturnCode = IFACE_OK;

		return retVal;
	}

	if (pReturnCode)
		*pReturnCode = IFACE_FAILED;

	return nullptr;
}
