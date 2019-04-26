#pragma once

#include <TF2Vulkan/IVulkanQueue.h>
#include <TF2Vulkan/Util/Checked.h>

#include <shaderapi/IShaderDevice.h>

#include <optional>

namespace TF2Vulkan
{
	class IShaderDeviceInternal : public IShaderDevice
	{
	public:
		struct VulkanInitData
		{
			uint32_t m_DeviceIndex = uint32_t(-1);
			vk::UniqueDevice m_Device;
			uint32_t m_GraphicsQueueIndex = uint32_t(-1);
			std::optional<uint32_t> m_TransferQueueIndex;
		};

		virtual void VulkanInit(VulkanInitData&& data) = 0;

		virtual const vk::Device& GetVulkanDevice() = 0;
		virtual vma::UniqueAllocator& GetVulkanAllocator() = 0;

		virtual const IVulkanQueue& GetGraphicsQueue() = 0;

		virtual Util::CheckedPtr<const IVulkanQueue> GetTransferQueue() = 0;
	};

	extern IShaderDeviceInternal& g_ShaderDevice;
}
