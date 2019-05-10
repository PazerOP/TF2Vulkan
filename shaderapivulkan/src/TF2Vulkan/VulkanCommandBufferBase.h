#pragma once
#include "interface/internal/IVulkanCommandBuffer.h"

namespace TF2Vulkan
{
	class VulkanCommandBufferWrapper final : public IVulkanCommandBuffer
	{
	public:
		VulkanCommandBufferWrapper(const vk::CommandBuffer& buffer, IVulkanQueue& queue) :
			m_Buffer(buffer), m_Queue(&queue)
		{
		}

		IVulkanQueue& GetQueue() override { return *m_Queue; }

	protected:
		const vk::CommandBuffer& GetCmdBuffer() const override { return m_Buffer; }

	private:
		vk::CommandBuffer m_Buffer;
		IVulkanQueue* m_Queue;
	};
}
