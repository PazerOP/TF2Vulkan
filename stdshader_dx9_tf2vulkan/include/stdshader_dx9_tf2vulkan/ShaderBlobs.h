#pragma once

namespace TF2Vulkan
{
	enum class ShaderBlob
	{
		Bik_VS,
		Bik_PS,
		BufferClearObeyStencil_VS,
		BufferClearObeyStencil_PS,
		Fillrate_VS,
		Fillrate_PS,
		XLitGeneric_VS,
		XLitGeneric_PS,
	};

	bool GetShaderBlob(ShaderBlob type, const void*& data, size_t& size);
}
