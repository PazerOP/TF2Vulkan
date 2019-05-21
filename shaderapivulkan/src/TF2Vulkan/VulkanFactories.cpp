#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IShaderAPITexture.h"
#include "FormatInfo.h"
#include "VulkanFactories.h"
#include "VulkanUtil.h"

using namespace TF2Vulkan;
using namespace TF2Vulkan::Factories;

static const auto MAP_REQUIRED_FLAGS = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
static const auto& MAP_REQUIRED_FLAGS_VK = *reinterpret_cast<const VkMemoryPropertyFlags*>(&MAP_REQUIRED_FLAGS);

template<typename T>
T& BaseObjectFactory<T>::SetDebugName(std::string&& dbgName)
{
	m_DebugName = std::move(dbgName);
	return *static_cast<T*>(this);
}

template<typename T>
T& BaseObjectFactory<T>::SetDebugName(const std::string_view& dbgName)
{
	return SetDebugName(std::string(dbgName));
}

template<typename T>
T& VmaObjectFactory<T>::SetAllocCreateInfo(const vma::AllocationCreateInfo& allocInfo)
{
	m_AllocInfo = allocInfo;
	return *static_cast<T*>(this);
}

template<typename T>
T& VmaObjectFactory<T>::SetMemoryType(vma::MemoryType type)
{
	m_AllocInfo.usage = (VmaMemoryUsage)type;
	return *static_cast<T*>(this);
}

template<typename T>
T& VmaObjectFactory<T>::SetAllowMapping(bool allow)
{
	if (allow)
	{
		m_AllocInfo.requiredFlags |= MAP_REQUIRED_FLAGS_VK;
		m_AllocInfo.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
	}
	else
	{
		m_AllocInfo.flags &= ~VMA_ALLOCATION_CREATE_MAPPED_BIT;
	}

	return *static_cast<T*>(this);
}

template struct BaseObjectFactory<BufferFactory>;
template struct BaseObjectFactory<ImageFactory>;
template struct VmaObjectFactory<BufferFactory>;
template struct VmaObjectFactory<ImageFactory>;

BufferFactory& BufferFactory::SetUsage(const vk::BufferUsageFlags& usage)
{
	m_CreateInfo.usage = usage;
	return *this;
}

BufferFactory& BufferFactory::SetSize(size_t size)
{
	m_CreateInfo.size = size;
	return *this;
}

BufferFactory& BufferFactory::SetInitialData(const void* initialData, size_t initialDataSize,
	size_t writeOffset)
{
	// TODO: Use a staging buffer instead of forcing the whole resource to be mappable forever
	SetAllowMapping(true);

	m_InitialData = initialData;
	m_InitialDataSize = initialDataSize;
	m_InitialDataWriteOffset = writeOffset;

	if (m_CreateInfo.size == 0)
		m_CreateInfo.size = m_InitialDataSize + m_InitialDataWriteOffset;

	return *this;
}

BufferFactory& BufferFactory::SetMemoryRequiredFlags(const vk::MemoryPropertyFlags& flags)
{
	m_AllocInfo.requiredFlags = (VkMemoryPropertyFlags)flags;
	return *this;
}

BufferFactory& BufferFactory::SetDebugName(std::string&& dbgName)
{
	m_DebugName = std::move(dbgName);
	return *this;
}

vma::AllocatedBuffer BufferFactory::Create() const
{
	const bool hasInitialData = m_InitialData && m_InitialDataSize > 0;

	if (hasInitialData)
	{
		// TODO: Create a staging buffer etc right here.
		assert((m_AllocInfo.requiredFlags & MAP_REQUIRED_FLAGS_VK) == MAP_REQUIRED_FLAGS_VK);
	}

	assert(m_CreateInfo.size > 0);
	auto created = g_ShaderDevice.GetVulkanAllocator().createBufferUnique(m_CreateInfo, m_AllocInfo);

	if (!m_DebugName.empty())
		g_ShaderDevice.SetDebugName(created.GetBuffer(), m_DebugName.c_str());

	if (hasInitialData)
		created.GetAllocation().Write(m_InitialData, m_InitialDataSize, m_InitialDataWriteOffset);

	return created;
}

ImageFactory& ImageFactory::SetCreateInfo(const vk::ImageCreateInfo& createInfo)
{
	m_CreateInfo = createInfo;
	return *this;
}

ImageFactory& ImageFactory::SetAllowMapping(bool allow)
{
	VmaObjectFactory<ImageFactory>::SetAllowMapping(allow);

	if (allow)
	{
		m_CreateInfo.tiling = vk::ImageTiling::eLinear;
	}

	return *this;
}

vma::AllocatedImage ImageFactory::Create() const
{
	auto created = g_ShaderDevice.GetVulkanAllocator().createImageUnique(m_CreateInfo, m_AllocInfo);

	if (!m_DebugName.empty())
		g_ShaderDevice.SetDebugName(created.GetImage(), m_DebugName.c_str());

	return created;
}

ImageMemoryBarrierFactory::ImageMemoryBarrierFactory()
{
	auto& srr = m_Barrier.subresourceRange;
	srr.layerCount = 1;
	srr.levelCount = 1;
}

static vk::AccessFlags GetAccessFlags(vk::PipelineStageFlags stage, bool read, bool write)
{
	using PSF = vk::PipelineStageFlagBits;
	using AF = vk::AccessFlagBits;

	static constexpr auto EMPTY = AF(0);

	switch ((vk::PipelineStageFlagBits)(VkPipelineStageFlags)stage)
	{
	default:
		assert(!"Unknown stage");
	case PSF::eAllGraphics:
		break;

	case PSF::eTransfer:
		return (read ? AF::eTransferRead : EMPTY) | (write ? AF::eTransferWrite : EMPTY);

	case PSF::eFragmentShader:
		return (read ? AF::eShaderRead : EMPTY) | (write ? AF::eShaderWrite : EMPTY);
	case PSF::eColorAttachmentOutput:
		return (read ? AF::eColorAttachmentRead : EMPTY) | (write ? AF::eColorAttachmentWrite : EMPTY);
	}

	return EMPTY;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetProducerStage(
	vk::PipelineStageFlags stage, bool read, bool write)
{
	m_SrcStage = stage;
	m_Barrier.srcAccessMask = GetAccessFlags(stage, read, write);
	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetConsumerStage(
	vk::PipelineStageFlags stage, bool read, bool write)
{
	m_DstStage = stage;
	m_Barrier.dstAccessMask = GetAccessFlags(stage, read, write);
	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetOldLayout(vk::ImageLayout oldLayout)
{
	m_Barrier.oldLayout = oldLayout;
	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetNewLayout(vk::ImageLayout newLayout)
{
	m_Barrier.newLayout = newLayout;
	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetImage(vk::Image image)
{
	m_Barrier.image = image;
	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetImage(
	const IVulkanTexture& image, bool setAspects, bool setFullSubresourceRange)
{
	m_Barrier.image = image.GetImage();

	if (setAspects || setFullSubresourceRange)
	{
		const auto& ci = image.GetImageCreateInfo();
		auto& srr = m_Barrier.subresourceRange;

		if (setAspects)
			srr.aspectMask = TF2Vulkan::FormatInfo::GetAspects(ci.format);

		if (setFullSubresourceRange)
		{
			srr.baseArrayLayer = 0;
			srr.baseMipLevel = 0;
			srr.layerCount = ci.arrayLayers;
			srr.levelCount = ci.mipLevels;
		}
	}


	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetMipLevels(uint32_t baseMipLevel, uint32_t mipLevelCount)
{
	auto& srr = m_Barrier.subresourceRange;
	srr.baseMipLevel = baseMipLevel;
	srr.levelCount = mipLevelCount;

	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetArrayLayers(uint32_t baseLayer, uint32_t layerCount)
{
	auto& srr = m_Barrier.subresourceRange;
	srr.baseArrayLayer = baseLayer;
	srr.layerCount = layerCount;

	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::Submit(IVulkanCommandBuffer& cmdBuf)
{
	return const_cast<ImageMemoryBarrierFactory&>(const_cast<const ImageMemoryBarrierFactory*>(this)->Submit(cmdBuf));
}

const ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::Submit(IVulkanCommandBuffer& cmdBuf) const
{
	cmdBuf.pipelineBarrier(m_SrcStage, m_DstStage, {}, {}, {}, m_Barrier);
	return *this;
}
