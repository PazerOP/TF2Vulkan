
#include "TF2Vulkan/MaterialSystem.h"

#include <materialsystem/imaterialsystem.h>

extern "C" __declspec(dllexport) void* CreateInterface(const char* pName, int* pReturnCode)
{
	if (!strcmp(pName, MATERIAL_SYSTEM_INTERFACE_VERSION))
	{
		auto retVal = TF2Vulkan::CreateMaterialSystem();
		if (pReturnCode)
			*pReturnCode = IFACE_OK;

		return retVal.release();
	}

	if (pReturnCode)
		*pReturnCode = IFACE_FAILED;

	return nullptr;
}
