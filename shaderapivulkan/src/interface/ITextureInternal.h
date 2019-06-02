#pragma once

#include <materialsystem/itexture.h>

class ITextureInternal : public ITexture
{
private:
	virtual void EXPAND_CONCAT(DummyFunc, __COUNTER__)() const final { NOT_IMPLEMENTED_FUNC(); }
public:
	virtual ShaderAPITextureHandle_t GetTextureHandle(int frame, int something = 0) = 0;
};
