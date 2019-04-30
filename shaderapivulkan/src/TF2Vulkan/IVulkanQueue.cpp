#include "stdafx.h"
#include "IVulkanQueue.h"

using namespace TF2Vulkan;

vk::UniqueCommandBuffer IVulkanQueue::CreateCmdBuffer() const
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = GetCmdPool();
	allocInfo.commandBufferCount = 1;

	auto allocated = GetDevice().allocateCommandBuffersUnique(allocInfo);
	ENSURE(!allocated.empty());
	return std::move(allocated[0]);
}

vk::UniqueCommandBuffer IVulkanQueue::CreateCmdBufferAndBegin(const vk::CommandBufferUsageFlags& beginFlags) const
{
	auto buf = CreateCmdBuffer();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = beginFlags;

	buf->begin(beginInfo);

	return buf;
}

void IVulkanQueue::EndAndSubmit(const vk::CommandBuffer& buf) const
{
	buf.end();
	Submit(buf);
}

void IVulkanQueue::Submit(const vk::CommandBuffer& buf) const
{
	vk::SubmitInfo submit;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &buf;

	auto& queue = GetQueue();
	queue.submit(submit, nullptr);
	queue.waitIdle();
}
