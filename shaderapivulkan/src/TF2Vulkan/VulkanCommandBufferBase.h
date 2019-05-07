#pragma once
#include "interface/internal/IVulkanCommandBuffer.h"

namespace TF2Vulkan
{
	class VulkanCommandBufferBase : public IVulkanCommandBuffer
	{
	public:
		void AddResource(vk::UniqueBuffer&& buffer) override final;
		void AddResource(vma::AllocatedBuffer&& buffer) override final;
		void AddResource(vk::UniqueDescriptorSet&& descriptorSet) override final;

		struct IResource
		{
			virtual ~IResource() = default;
			std::unique_ptr<IResource> m_Next;
		};

	private:
		void ReleaseAttachedResources() override final;

		struct BufferNode;
		struct AllocatedBufferNode;
		struct DescriptorSetNode;

		void AddResource(std::unique_ptr<IResource>&& node);

		std::unique_ptr<IResource> m_FirstNode;
	};

	class VulkanCommandBufferWrapper final : public VulkanCommandBufferBase
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
