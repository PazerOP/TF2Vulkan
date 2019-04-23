#include "MaterialSystemHardwareConfig.h"

#include <materialsystem/imaterialsystemhardwareconfig.h>

using namespace TF2Vulkan;

namespace
{
	class MaterialSystemHardwareConfig : public IMaterialSystemHardwareConfig
	{
	public:

	};
}

static MaterialSystemHardwareConfig s_HardwareConfig;
IMaterialSystemHardwareConfig* TF2Vulkan::GetMaterialSystemHardwareConfig()
{
	return &s_HardwareConfig;
}
