#pragma once

#include <materialsystem/imaterialsystemhardwareconfig.h>

namespace TF2Vulkan
{
	class IMaterialSystemHardwareConfigInternal : public IMaterialSystemHardwareConfig
	{
	public:
		virtual void Init() = 0;
		virtual bool NeedsShaderSRGBConversionImpl() const = 0;

		int NeedsShaderSRGBConversion() const override final
		{
			return NeedsShaderSRGBConversionImpl() ? 1 : 0;
		}

		virtual uint32_t MaxVertexAttributes() const = 0;
	};

	extern IMaterialSystemHardwareConfigInternal& g_MatSysConfig;
}
