#pragma once

namespace TF2Vulkan
{
	enum class ShaderBlobType
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

	class IShaderBlobs
	{
	public:
		virtual bool GetShaderBlob(ShaderBlobType type, const void*& data, size_t& size) const = 0;
	};

#define SHADER_BLOBS_INTERFACE_VERSION "TF2Vulkan_IShaderBlobs001"
	extern const IShaderBlobs* g_ShaderBlobs;
}
