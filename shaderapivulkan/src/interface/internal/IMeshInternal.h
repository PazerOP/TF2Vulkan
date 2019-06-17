#pragma once

#include <materialsystem/imesh.h>

namespace TF2Vulkan
{
	class IBufferCommonInternal
	{
	public:
		virtual ~IBufferCommonInternal() = default;

		virtual void GetGPUBuffer(vk::Buffer& buffer, size_t& offset) = 0;
	};

	class IVertexBufferInternal : public IVertexBuffer, public IBufferCommonInternal
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

	class IIndexBufferInternal : public IIndexBuffer, public IBufferCommonInternal
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

	class IMeshInternal : public IMesh
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
		virtual VertexFormat GetColorMeshFormat() const = 0;

		virtual const IIndexBufferInternal& GetIndexBuffer() const = 0;
		virtual const IVertexBufferInternal& GetVertexBuffer() const = 0;

		IIndexBufferInternal& GetIndexBuffer();
		IVertexBufferInternal& GetVertexBuffer();
	};

	inline IIndexBufferInternal& IMeshInternal::GetIndexBuffer()
	{
		return const_cast<IIndexBufferInternal&>(Util::as_const(this)->GetIndexBuffer());
	}

	inline IVertexBufferInternal& IMeshInternal::GetVertexBuffer()
	{
		return const_cast<IVertexBufferInternal&>(Util::as_const(this)->GetVertexBuffer());
	}
}
