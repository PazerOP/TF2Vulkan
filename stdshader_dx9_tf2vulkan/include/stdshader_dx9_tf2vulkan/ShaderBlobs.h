#pragma once

namespace TF2Vulkan
{
	enum class ShaderBlob
	{
		VertexLitAndUnlitGeneric_VS,
		VertexLitAndUnlitGeneric_PS,
	};

	bool GetShaderBlob(ShaderBlob type, const void*& data, size_t& size);
}
