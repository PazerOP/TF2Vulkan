#pragma once

#include <materialsystem/imaterialsystemhardwareconfig.h>

namespace TF2Vulkan
{
	class IMaterialSystemHardwareConfigInternal : public IMaterialSystemHardwareConfig
	{
	public:
		virtual bool NeedsShaderSRGBConversionImpl() const = 0;

		int NeedsShaderSRGBConversion() const override final
		{
			return NeedsShaderSRGBConversionImpl() ? 1 : 0;
		}
	};
}
