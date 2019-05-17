#pragma once

#include <materialsystem/imaterialsystemhardwareconfig.h>

namespace TF2Vulkan
{
	class IMaterialSystemHardwareConfigInternal : public IMaterialSystemHardwareConfig
	{
	private:
		// These are here to catch any potentially missing virtual functions
		virtual void DummyFunc0() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc1() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc2() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc3() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc4() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc5() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc6() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc7() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc8() const final { NOT_IMPLEMENTED_FUNC(); }
		virtual void DummyFunc9() const final { NOT_IMPLEMENTED_FUNC(); }

	public:
		//virtual void Init() = 0;
		virtual bool NeedsShaderSRGBConversionImpl() const = 0;

		int NeedsShaderSRGBConversion() const override final
		{
			return NeedsShaderSRGBConversionImpl() ? 1 : 0;
		}

		virtual uint32_t MaxVertexAttributes() const = 0;
	};

	extern IMaterialSystemHardwareConfigInternal& g_MatSysConfig;
}
