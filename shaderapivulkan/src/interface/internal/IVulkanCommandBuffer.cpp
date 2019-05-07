#include "IVulkanCommandBuffer.h"
#include "IVulkanQueue.h"
#include "TF2Vulkan/ShaderDeviceMgr.h"
#include "TF2Vulkan/VulkanUtil.h"

using namespace TF2Vulkan;

static constexpr Color PIX_TF2VULKAN_RED(255, 93, 79);
static vk::DebugUtilsLabelEXT InitDebugUtilsLabel(const char* name, const Color& color = PIX_TF2VULKAN_RED)
{
	vk::DebugUtilsLabelEXT label;

	label.color[0] = color.r() / 255.0f;
	label.color[1] = color.g() / 255.0f;
	label.color[2] = color.b() / 255.0f;
	label.color[3] = color.a() / 255.0f;

	label.pLabelName = name;

	return label;
}

void IVulkanCommandBuffer::insertDebugUtilsLabelEXT(const vk::DebugUtilsLabelEXT& labelInfo)
{
	return GetCmdBuffer().insertDebugUtilsLabelEXT(labelInfo, g_ShaderDeviceMgr.GetDynamicDispatch());
}

void IVulkanCommandBuffer::beginDebugUtilsLabelEXT(const vk::DebugUtilsLabelEXT& labelInfo)
{
	assert(m_DebugScopeCount >= 0);
	m_DebugScopeCount++;
	return GetCmdBuffer().beginDebugUtilsLabelEXT(labelInfo, g_ShaderDeviceMgr.GetDynamicDispatch());
}

void IVulkanCommandBuffer::endDebugUtilsLabelEXT()
{
	// It's actually a pretty common thing to have this get called after
	// the cmd buffer has been ended. For example, if you make a PixScope
	// object, then call Submit() at the end of that scope, the PixScope
	// will call this function after the Submit() has happened.
	if (m_DebugScopeCount == 0)
		return;

	m_DebugScopeCount--;
	assert(m_DebugScopeCount >= 0);
	return GetCmdBuffer().endDebugUtilsLabelEXT(g_ShaderDeviceMgr.GetDynamicDispatch());
}

void IVulkanCommandBuffer::begin(const vk::CommandBufferBeginInfo& beginInfo)
{
	assert(!m_IsActive);
	m_IsActive = true;
	return GetCmdBuffer().begin(beginInfo);
}

void IVulkanCommandBuffer::beginRenderPass(const vk::RenderPassBeginInfo& renderPassBegin, const vk::SubpassContents& contents)
{
	assert(!m_ActiveRenderPass);
	m_ActiveRenderPass.emplace(ActiveRenderPass{ renderPassBegin, contents });
	return GetCmdBuffer().beginRenderPass(renderPassBegin, contents);
}

void IVulkanCommandBuffer::bindDescriptorSets(const vk::PipelineBindPoint& pipelineBindPoint, const vk::PipelineLayout& layout, uint32_t firstSet, const vk::ArrayProxy<const vk::DescriptorSet>& descriptorSets, const vk::ArrayProxy<const uint32_t> dynamicOffsets)
{
	return GetCmdBuffer().bindDescriptorSets(pipelineBindPoint, layout, firstSet, descriptorSets, dynamicOffsets);
}

void IVulkanCommandBuffer::bindPipeline(const vk::PipelineBindPoint& pipelineBindPoint, const vk::Pipeline& pipeline)
{
	return GetCmdBuffer().bindPipeline(pipelineBindPoint, pipeline);
}

void IVulkanCommandBuffer::bindIndexBuffer(const vk::Buffer& buffer,
	const vk::DeviceSize& offset, const vk::IndexType& indexType)
{
	return GetCmdBuffer().bindIndexBuffer(buffer, offset, indexType);
}

void IVulkanCommandBuffer::bindVertexBuffers(uint32_t firstBinding,
	const vk::ArrayProxy<const vk::Buffer>& buffers, const vk::ArrayProxy<const vk::DeviceSize>& offsets)
{
	return GetCmdBuffer().bindVertexBuffers(firstBinding, buffers, offsets);
}

void IVulkanCommandBuffer::copyBufferToImage(const vk::Buffer& buf, const vk::Image& img,
	const vk::ImageLayout& dstImageLayout, const vk::ArrayProxy<const vk::BufferImageCopy>& regions)
{
	return GetCmdBuffer().copyBufferToImage(buf, img, dstImageLayout, regions);
}

void IVulkanCommandBuffer::clearAttachments(uint32_t attachmentCount, const vk::ClearAttachment* pAttachments,
	uint32_t rectCount, const vk::ClearRect* pRects)
{
	return GetCmdBuffer().clearAttachments(attachmentCount, pAttachments, rectCount, pRects);
}

void IVulkanCommandBuffer::clearAttachments(const vk::ArrayProxy<const vk::ClearAttachment>& attachments,
	const vk::ArrayProxy<const vk::ClearRect>& rects)
{
	return GetCmdBuffer().clearAttachments(attachments, rects);
}

void IVulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
	int32_t vertexOffset, uint32_t firstInstance)
{
	return GetCmdBuffer().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void IVulkanCommandBuffer::end()
{
	while (m_DebugScopeCount > 0)
		endDebugUtilsLabelEXT();

	assert(m_IsActive);
	m_IsActive = false;
	return GetCmdBuffer().end();
}

void IVulkanCommandBuffer::endRenderPass()
{
	assert(m_ActiveRenderPass);
	m_ActiveRenderPass.reset();
	return GetCmdBuffer().endRenderPass();
}

void IVulkanCommandBuffer::pipelineBarrier(const vk::PipelineStageFlags& srcStageMask,
	const vk::PipelineStageFlags& dstStageMask, const vk::DependencyFlags& dependencyFlags,
	const vk::ArrayProxy<const vk::MemoryBarrier>& memoryBarriers,
	const vk::ArrayProxy<const vk::BufferMemoryBarrier>& bufferMemoryBarriers,
	const vk::ArrayProxy<const vk::ImageMemoryBarrier>& imageMemoryBarriers)
{
	return GetCmdBuffer().pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags,
		memoryBarriers, bufferMemoryBarriers, imageMemoryBarriers);
}

bool IVulkanCommandBuffer::IsActive() const
{
	return m_IsActive;
}

void IVulkanCommandBuffer::Submit(vk::SubmitInfo submitInfo, const vk::Fence& fence)
{
	if (IsActive())
		end();

	auto& buf = GetCmdBuffer();

	assert(!submitInfo.pCommandBuffers);
	submitInfo.pCommandBuffers = &GetCmdBuffer();
	submitInfo.commandBufferCount = 1;

	auto& q = GetQueue().GetQueue();
	q.submit(submitInfo, fence);
	q.waitIdle(); // FIXME

	ReleaseAttachedResources();
}

void IVulkanCommandBuffer::CopyBufferToImage(const vk::Buffer& buffer, const vk::Image& image,
	const vk::Extent2D& size, uint32_t sliceOffset)
{
	vk::BufferImageCopy copy;

	copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copy.imageSubresource.layerCount = 1;
	copy.imageSubresource.baseArrayLayer = sliceOffset;
	copy.imageExtent = ToExtent3D(size);

	copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copy);
}

void IVulkanCommandBuffer::InsertDebugLabel(const Color& color, const char* text)
{
	insertDebugUtilsLabelEXT(InitDebugUtilsLabel(text, color));
}

void IVulkanCommandBuffer::InsertDebugLabel(const char* text)
{
	insertDebugUtilsLabelEXT(InitDebugUtilsLabel(text));
}

PixScope IVulkanCommandBuffer::DebugRegionBegin(const Color& color, const char* text)
{
	beginDebugUtilsLabelEXT(InitDebugUtilsLabel(text, color));
	return PixScope(*this);
}

PixScope IVulkanCommandBuffer::DebugRegionBegin(const char* text)
{
	beginDebugUtilsLabelEXT(InitDebugUtilsLabel(text));
	return PixScope(*this);
}

auto IVulkanCommandBuffer::GetActiveRenderPass() const -> const ActiveRenderPass*
{
	if (m_ActiveRenderPass)
		return &*m_ActiveRenderPass;
	else
		return nullptr;
}

bool IVulkanCommandBuffer::IsRenderPassActive(const vk::RenderPassBeginInfo& beginInfo, const vk::SubpassContents& contents) const
{
	if (!m_ActiveRenderPass)
		return false;

	auto& activeRP = *m_ActiveRenderPass;

	// All the simple comparisons first
	if (contents != activeRP.m_Contents ||
		beginInfo.renderPass != activeRP.m_BeginInfo.renderPass ||
		beginInfo.framebuffer != activeRP.m_BeginInfo.framebuffer ||
		beginInfo.renderArea != activeRP.m_BeginInfo.renderArea ||
		beginInfo.clearValueCount != activeRP.m_BeginInfo.clearValueCount)
	{
		return false;
	}

	// Already checked above to make sure both clearValueCounts are equal
	for (uint32_t i = 0; i < beginInfo.clearValueCount; i++)
	{
		const auto& a = beginInfo.pClearValues[i];
		const auto& b = activeRP.m_BeginInfo.pClearValues[i];
		if (!IsEqual(a, b, ClearValueType::Float))
			return false;
	}

	return true;
}
