#pragma once

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

		[[nodiscard]] vk::UniqueCommandBuffer CreateCmdBuffer() const;
		[[nodiscard]] vk::UniqueCommandBuffer CreateCmdBufferAndBegin(
			const vk::CommandBufferUsageFlags& beginFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit) const;

		void EndAndSubmit(const vk::CommandBuffer& buf) const;
		void Submit(const vk::CommandBuffer& buf) const;
	};
}
