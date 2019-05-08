#pragma once
#include <materialsystem/idebugtextureinfo.h>

namespace TF2Vulkan
{
	class IDebugTextureInfoInternal : public IDebugTextureInfo
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

	};
}
