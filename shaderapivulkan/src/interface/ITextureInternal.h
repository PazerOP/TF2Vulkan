#pragma once

#include <materialsystem/itexture.h>

class ITextureInternal : public ITexture
{
public:
	// Untested functions
	[[deprecated("Untested")]] virtual void Bind(Sampler_t) = 0;
	[[deprecated("Untested")]] virtual void Bind(Sampler_t, int, Sampler_t) = 0;
	[[deprecated("Untested")]] virtual int GetReferenceCount() const = 0;
	[[deprecated("Untested")]] virtual void GetReflectivity(Vector&) const = 0;
	[[deprecated("Untested")]] virtual void SetRenderTarget(int) = 0;
	[[deprecated("Untested")]] virtual void ReleaseMemory() = 0;
	[[deprecated("Untested")]] virtual void OnRestore() = 0;
	[[deprecated("Untested")]] virtual void SetFilteringAndClampingMode(bool) = 0;
	[[deprecated("Untested")]] virtual void Precache() = 0;
	[[deprecated("Untested")]] virtual void CopyFrameBufferToMe(int, Rect_t*, Rect_t*) = 0;
	[[deprecated("Untested")]] virtual void CopyMeToFrameBuffer(int, Rect_t*, Rect_t*) = 0;
	[[deprecated("Untested")]] virtual void GetEmbeddedTexture(int) = 0;

	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual ShaderAPITextureHandle_t GetTextureHandle(int frame, int something = 0) const = 0;

private:
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
};
