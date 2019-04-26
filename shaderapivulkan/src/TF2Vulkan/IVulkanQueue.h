#pragma once

namespace TF2Vulkan
{
	class IVulkanQueue
	{
	protected:
		~IVulkanQueue() = default;

	public:
		virtual const vk::Queue& GetQueue() const = 0;
		virtual const vk::CommandPool& GetCmdPool() const = 0;
	};
}
