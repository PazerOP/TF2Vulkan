#pragma once

namespace TF2Vulkan
{
	class IVulkanCommandBuffer
	{
	public:
		virtual void AddResource(vk::UniqueBuffer&& buffer) = 0;

		virtual const vk::CommandBuffer& GetCmdBuffer() const = 0;

		const vk::CommandBuffer* operator->() const { return &GetCmdBuffer(); }

	protected:
		virtual ~IVulkanCommandBuffer() = default;
	};
}
