#pragma once

#include <materialsystem/imaterialsystemhardwareconfig.h>

namespace TF2Vulkan
{
	class IMaterialSystemHardwareConfigInternal : public IMaterialSystemHardwareConfig
	{
	public:
		virtual const char* GetHWSpecificShaderDLLName() const = 0;
	};
}
