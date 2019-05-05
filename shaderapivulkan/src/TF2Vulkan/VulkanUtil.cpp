#include "TF2Vulkan/VulkanUtil.h"
#include "ShaderDevice.h"
#include "ShaderDeviceMgr.h"

using namespace TF2Vulkan;

void TF2Vulkan::TransitionImageLayout(const vk::Image& image, const vk::Format& format,
	const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
	const vk::CommandBuffer& cmdBuf, uint32_t mipLevel)
{
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;

	auto& srr = barrier.subresourceRange;
	srr.aspectMask = vk::ImageAspectFlagBits::eColor;
	srr.baseMipLevel = mipLevel;
	srr.levelCount = 1;
	srr.baseArrayLayer = 0;
	srr.layerCount = 1;

	vk::PipelineStageFlags srcStageMask;
	vk::PipelineStageFlags dstStageMask;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;

		dstStageMask = vk::PipelineStageFlagBits::eTransfer;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;

		dstStageMask = vk::PipelineStageFlagBits::eTransfer;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		srcStageMask = vk::PipelineStageFlagBits::eTransfer;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;

		dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
	}
	else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		//barrier.dstAccessMask = vk::AccessFlagBits::e;
	}
	else if (oldLayout == vk::ImageLayout::ePresentSrcKHR && newLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		//barrier.srcAccessMask = vk::AccessFlagBits::

		dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
		//barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
	}
	else
	{
		throw VulkanException("Unsupported layout transition", EXCEPTION_DATA());
	}

	cmdBuf.pipelineBarrier(
		srcStageMask,
		dstStageMask,
		vk::DependencyFlags{}, {}, {}, barrier);
}

vk::Extent2D TF2Vulkan::ToExtent2D(const vk::Extent3D& extent)
{
	assert(extent.depth == 1);
	return vk::Extent2D(extent.width, extent.height);
}

static vk::DebugUtilsLabelEXT InitDebugUtilsLabel(const char* name, const Color& color)
{
	vk::DebugUtilsLabelEXT label;

	label.color[0] = color.r() / 255.0f;
	label.color[1] = color.g() / 255.0f;
	label.color[2] = color.b() / 255.0f;
	label.color[3] = color.a() / 255.0f;

	label.pLabelName = name;

	return label;
}

void TF2Vulkan::SetPIXMarker(const vk::CommandBuffer& cmdBuf, const char* name, const Color& color)
{
	cmdBuf.insertDebugUtilsLabelEXT(InitDebugUtilsLabel(name, color), g_ShaderDeviceMgr.GetDynamicDispatch());
}

void TF2Vulkan::SetPIXMarker(const vk::Queue& queue, const char* name, const Color& color)
{
	queue.insertDebugUtilsLabelEXT(InitDebugUtilsLabel(name, color), g_ShaderDeviceMgr.GetDynamicDispatch());
}

void TF2Vulkan::BeginPIXEvent(const vk::CommandBuffer& cmdBuf, const char* name, const Color& color)
{
	cmdBuf.beginDebugUtilsLabelEXT(InitDebugUtilsLabel(name, color), g_ShaderDeviceMgr.GetDynamicDispatch());
}

void TF2Vulkan::BeginPIXEvent(const vk::Queue& queue, const char* name, const Color& color)
{
	queue.beginDebugUtilsLabelEXT(InitDebugUtilsLabel(name, color), g_ShaderDeviceMgr.GetDynamicDispatch());
}

void TF2Vulkan::EndPIXEvent(const vk::CommandBuffer& cmdBuf)
{
	cmdBuf.endDebugUtilsLabelEXT(g_ShaderDeviceMgr.GetDynamicDispatch());
}

void TF2Vulkan::EndPIXEvent(const vk::Queue& queue)
{
	queue.endDebugUtilsLabelEXT(g_ShaderDeviceMgr.GetDynamicDispatch());
}

PixScope::~PixScope()
{
	std::visit([](const auto & obj)
		{
			using type = std::decay_t<decltype(obj)>;
			if constexpr (std::is_same_v<type, std::monostate>)
				TF2Vulkan::EndPIXEvent(g_ShaderDevice.GetGraphicsQueue().GetQueue());
			else
				TF2Vulkan::EndPIXEvent(obj);

		}, m_Object);
}

void PixScope::BeginPIXEventAnon(const char* fmt, const Color& color)
{
	const auto& queue = g_ShaderDevice.GetGraphicsQueue().GetQueue();
	m_Object = queue;
	BeginPIXEvent(queue, fmt, color);
}
