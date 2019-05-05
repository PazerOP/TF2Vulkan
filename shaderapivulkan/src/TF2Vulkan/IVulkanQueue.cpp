#include "stdafx.h"
#include "ShaderDevice.h"
#include "IVulkanQueue.h"

#include <atomic>

using namespace TF2Vulkan;

static std::atomic<size_t> s_CmdBufIndex = 1;

vk::UniqueCommandBuffer IVulkanQueue::CreateCmdBuffer() const
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = GetCmdPool();
	allocInfo.commandBufferCount = 1;

	auto allocated = GetDevice().allocateCommandBuffersUnique(allocInfo);

	for (auto& newBuf : allocated)
	{
		char nameBuf[128];
		sprintf_s(nameBuf, "TF2Vulkan Command Buffer %zu", s_CmdBufIndex++);
		g_ShaderDevice.SetDebugName(newBuf, nameBuf);
	}

	ENSURE(!allocated.empty());
	return std::move(allocated[0]);
}

vk::UniqueCommandBuffer IVulkanQueue::CreateCmdBufferAndBegin(const vk::CommandBufferUsageFlags& beginFlags) const
{
	auto buf = CreateCmdBuffer();

	vk::CommandBufferInheritanceInfo inheritInfo;

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = beginFlags;
	beginInfo.pInheritanceInfo = &inheritInfo;

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
