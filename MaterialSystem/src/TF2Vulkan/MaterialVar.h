#pragma once

#include <materialsystem/imaterialvar.h>

namespace TF2Vulkan
{
	class MaterialVar final : public IMaterialVar
	{
	public:

		ITexture* GetTextureValue() override;
		const char* GetName() const { return m_Name.String(); }
	};
}
