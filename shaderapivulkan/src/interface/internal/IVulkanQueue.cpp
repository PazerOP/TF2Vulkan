#include "IVulkanQueue.h"
#include "TF2Vulkan/ShaderDevice.h"
#include "TF2Vulkan/VulkanCommandBufferBase.h"

#include <atomic>

using namespace TF2Vulkan;

static std::atomic<size_t> s_CmdBufIndex = 1;

namespace
{
#pragma warning(push)
#pragma warning(disable : 4996)
	class UniqueCmdBuffer final : public VulkanCommandBufferBase
	{
	public:
		IVulkanQueue& GetQueue() override { return *m_Queue; }

		vk::UniqueCommandBuffer m_Buffer;
		IVulkanQueue* m_Queue = nullptr;

	protected:
		const vk::CommandBuffer& GetCmdBuffer() const override { return m_Buffer.get(); }
	};
#pragma warning(pop)
}

static std::unique_ptr<UniqueCmdBuffer> CreateCmdBuffer(IVulkanQueue& queue)
{
	auto buf = std::make_unique<UniqueCmdBuffer>();
	buf->m_Queue = &queue;

	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = queue.GetCmdPool();
	allocInfo.commandBufferCount = 1;

	auto allocated = queue.GetDevice().allocateCommandBuffersUnique(allocInfo);

	for (auto& newBuf : allocated)
	{
		char nameBuf[128];
		sprintf_s(nameBuf, "TF2Vulkan Command Buffer %zu", s_CmdBufIndex++);
		g_ShaderDevice.SetDebugName(newBuf, nameBuf);
	}

	ENSURE(!allocated.empty());
	buf->m_Buffer = std::move(allocated[0]);

	return buf;
}

std::unique_ptr<IVulkanCommandBuffer> IVulkanQueue::CreateCmdBuffer()
{
	return ::CreateCmdBuffer(*this);
}

std::unique_ptr<IVulkanCommandBuffer> IVulkanQueue::CreateCmdBufferAndBegin(const vk::CommandBufferUsageFlags& beginFlags)
{
	auto buf = ::CreateCmdBuffer(*this);

	vk::CommandBufferInheritanceInfo inheritInfo;

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = beginFlags;
	//beginInfo.pInheritanceInfo = &inheritInfo;

	buf->begin(beginInfo);

	return buf;
}

#if false
void IVulkanQueue::Submit(const vk::CommandBuffer& buf, const vk::SubmitInfo& submitInfo, const vk::Fence& fence) const
{
	vk::SubmitInfo submit;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &buf;

	auto& queue = GetQueue();
	queue.submit(submit, nullptr);
	queue.waitIdle();
}
#endif
