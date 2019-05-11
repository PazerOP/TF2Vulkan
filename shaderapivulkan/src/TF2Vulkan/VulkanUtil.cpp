#include "TF2Vulkan/VulkanUtil.h"
#include "FormatInfo.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "ShaderDeviceMgr.h"

#include <TF2Vulkan/Util/Enums.h>

using namespace TF2Vulkan;

namespace
{
	enum class PackedLayoutSrcTarget : uint64_t;
}

static constexpr PackedLayoutSrcTarget PackImageLayout(const vk::ImageLayout& src, const vk::ImageLayout& dst)
{
	assert(Util::UValue(src) < (1ULL << 32));
	assert(Util::UValue(dst) < (1ULL << 32));

	return PackedLayoutSrcTarget((uint64_t(Util::UValue(src)) << 32) | uint64_t(Util::UValue(dst)) & ((1ULL << 32) - 1));
}

void TF2Vulkan::TransitionImageLayout(const vk::Image& image, const vk::Format& format,
	const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
	IVulkanCommandBuffer& cmdBuf, uint32_t mipLevel)
{
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;

	auto& srr = barrier.subresourceRange;
	srr.aspectMask = FormatInfo::GetAspects(format);
	srr.baseMipLevel = mipLevel;
	srr.levelCount = 1;
	srr.baseArrayLayer = 0;
	srr.layerCount = 1;

	vk::PipelineStageFlags srcStageMask;
	vk::PipelineStageFlags dstStageMask;

	if (oldLayout == vk::ImageLayout::eUndefined)
	{
		srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;

		switch (newLayout)
		{
		case vk::ImageLayout::eTransferDstOptimal:
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
			break;

		case vk::ImageLayout::ePresentSrcKHR:
			dstStageMask = vk::PipelineStageFlagBits::eTransfer;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			break;

		case vk::ImageLayout::eColorAttachmentOptimal:
			dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
			break;

		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			break;

		default:
			throw VulkanException("Unsupported layout transition", EXCEPTION_DATA());
		}
	}
	else
	{
		switch (PackImageLayout(oldLayout, newLayout))
		{
		case PackImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal):
			srcStageMask = vk::PipelineStageFlagBits::eTransfer;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			break;

		case PackImageLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR):
			srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
			//barrier.dstAccessMask = vk::AccessFlagBits::e;
			break;

		case PackImageLayout(vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal):
			srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
			//barrier.srcAccessMask = vk::AccessFlagBits::
			dstStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
			//barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
			break;

		default:
			throw VulkanException("Unsupported layout transition", EXCEPTION_DATA());
		}
	}

	cmdBuf.TryEndRenderPass();
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

vk::Extent3D TF2Vulkan::ToExtent3D(const vk::Extent2D& extent)
{
	return vk::Extent3D{ extent.width, extent.height, 1 };
}

bool TF2Vulkan::IsEqual(const vk::ClearValue& lhs, const vk::ClearValue& rhs, ClearValueType type)
{
	switch (type)
	{
	case ClearValueType::Float:
	case ClearValueType::Int:
	case ClearValueType::UInt:
		return IsEqual(lhs.color, rhs.color, Util::SafeConvert<ClearColorType>(type));

	case ClearValueType::DepthStencil:
		return (lhs.depthStencil == rhs.depthStencil);

	default:
		throw VulkanException("Invalid ClearValueType", EXCEPTION_DATA());
	}
}

bool TF2Vulkan::IsEqual(const vk::ClearColorValue& lhs, const vk::ClearColorValue& rhs, ClearColorType type)
{
	switch (type)
	{
	default:
		throw VulkanException("Invalid ClearColorType", EXCEPTION_DATA());

	case ClearColorType::UInt:
		return
			lhs.uint32[0] == rhs.uint32[0] &&
			lhs.uint32[1] == rhs.uint32[1] &&
			lhs.uint32[2] == rhs.uint32[2] &&
			lhs.uint32[3] == rhs.uint32[3];

	case ClearColorType::Int:
		return
			lhs.int32[0] == rhs.int32[0] &&
			lhs.int32[1] == rhs.int32[1] &&
			lhs.int32[2] == rhs.int32[2] &&
			lhs.int32[3] == rhs.int32[3];

	case ClearColorType::Float:
		return
			lhs.float32[0] == rhs.float32[0] &&
			lhs.float32[1] == rhs.float32[1] &&
			lhs.float32[2] == rhs.float32[2] &&
			lhs.float32[3] == rhs.float32[3];
	}
}

bool operator==(const vk::ClearColorValue& lhs, const vk::ClearColorValue& rhs)
{
	return
		lhs.float32[0] == rhs.float32[0] &&
		lhs.float32[1] == rhs.float32[1] &&
		lhs.float32[2] == rhs.float32[2] &&
		lhs.float32[3] == rhs.float32[3];
}

bool operator!=(const vk::ClearColorValue& lhs, const vk::ClearColorValue& rhs)
{
	return !operator==(lhs, rhs);
}
