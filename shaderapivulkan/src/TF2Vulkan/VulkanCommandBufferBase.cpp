#include "VulkanCommandBufferBase.h"

using namespace TF2Vulkan;

struct VulkanCommandBufferBase::BufferNode : IResource
{
	vk::UniqueBuffer m_Buffer;
};
struct VulkanCommandBufferBase::AllocatedBufferNode : IResource
{
	vma::AllocatedBuffer m_Buffer;
};
struct VulkanCommandBufferBase::DescriptorSetNode : IResource
{
	vk::UniqueDescriptorSet m_DescriptorSet;
};

void VulkanCommandBufferBase::AddResource(vk::UniqueBuffer&& buffer)
{
	auto newNode = std::make_unique<BufferNode>();
	newNode->m_Buffer = std::move(buffer);
	AddResource(std::move(newNode));
}

void VulkanCommandBufferBase::AddResource(vma::AllocatedBuffer&& buffer)
{
	auto newNode = std::make_unique<AllocatedBufferNode>();
	newNode->m_Buffer = std::move(buffer);
	AddResource(std::move(newNode));
}

void VulkanCommandBufferBase::AddResource(vk::UniqueDescriptorSet&& descriptorSet)
{
	auto newNode = std::make_unique<DescriptorSetNode>();
	newNode->m_DescriptorSet = std::move(descriptorSet);
	AddResource(std::move(newNode));
}

void VulkanCommandBufferBase::ReleaseAttachedResources()
{
	m_FirstNode.reset();
}

void VulkanCommandBufferBase::AddResource(std::unique_ptr<IResource>&& node)
{
	node->m_Next = std::move(m_FirstNode);
	m_FirstNode = std::move(node);
}
