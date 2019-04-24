#pragma once

#include <shaderapi/IShaderDevice.h>

namespace TF2Vulkan
{
	class IShaderDeviceInternal : public IShaderDevice
	{
	public:
		virtual void SetVulkanDevice(vk::UniqueDevice&& device) = 0;
		virtual vk::Device& GetVulkanDevice() = 0;
	};

	extern IShaderDeviceInternal& g_ShaderDevice;
}
