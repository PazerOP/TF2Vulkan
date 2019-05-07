#pragma once

#include "interface/internal/IVulkanCommandBuffer.h"

namespace TF2Vulkan
{
	class IVulkanQueue
	{
	protected:
		~IVulkanQueue() = default;

	public:
		virtual const vk::Device& GetDevice() const = 0;
		virtual const vk::Queue& GetQueue() const = 0;
		virtual const vk::CommandPool& GetCmdPool() const = 0;

		[[nodiscard]] std::unique_ptr<IVulkanCommandBuffer> CreateCmdBuffer();
		[[nodiscard]] std::unique_ptr<IVulkanCommandBuffer> CreateCmdBufferAndBegin(
			const vk::CommandBufferUsageFlags& beginFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	};
}
