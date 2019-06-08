#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "interface/internal/IShaderAPITexture.h"
#include "FormatInfo.h"
#include "VulkanFactories.h"
#include "VulkanUtil.h"

#include <TF2Vulkan/Util/platform.h>

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

ImageFactory::ImageFactory()
{
	m_CreateInfo.arrayLayers = 1;
	m_CreateInfo.mipLevels = 1;
}

ImageFactory& ImageFactory::SetCreateInfo(const vk::ImageCreateInfo& createInfo)
{
	m_CreateInfo = createInfo;
	return *this;
}

ImageFactory& ImageFactory::AddUsageFlags(const vk::ImageUsageFlags& usage)
{
	m_CreateInfo.usage |= usage;
	return *this;
}

ImageFactory& ImageFactory::SetUsage(const vk::ImageUsageFlags& usage)
{
	m_CreateInfo.usage = usage;
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

ImageFactory& ImageFactory::SetDefaultLayout(vk::ImageLayout layout)
{
	m_DefaultLayout = layout;
	return *this;
}

ImageFactory& ImageFactory::SetFormat(vk::Format format)
{
	m_CreateInfo.format = format;
	return *this;
}

ImageFactory& ImageFactory::SetExtent(uint32_t width, uint32_t height, uint32_t depth)
{
	m_CreateInfo.extent = { width, height, depth };
	m_CreateInfo.imageType = (depth == 1) ? vk::ImageType::e2D : vk::ImageType::e3D;

	return *this;
}

ImageFactory& ImageFactory::SetExtent(const vk::Extent2D& extent)
{
	return SetExtent(extent.width, extent.height);
}

vma::AllocatedImage ImageFactory::Create() const
{
	auto created = g_ShaderDevice.GetVulkanAllocator().createImageUnique(m_CreateInfo, m_AllocInfo);

	if (!m_DebugName.empty())
		g_ShaderDevice.SetDebugName(created.GetImage(), m_DebugName.c_str());

	if (m_DefaultLayout != m_CreateInfo.initialLayout)
	{
		auto cmdBuf = g_ShaderDevice.GetGraphicsQueue().CreateCmdBufferAndBegin();

		Factories::ImageMemoryBarrierFactory{}
			.SetImage(created.GetImage())
			.SetAspectsFromFormat(m_CreateInfo.format)
			.SetFullSubresourceRange(m_CreateInfo)
			.SetOldLayout(vk::ImageLayout::eUndefined)
			.SetNewLayout(m_DefaultLayout)
			.SetProducerStage(vk::PipelineStageFlagBits::eBottomOfPipe)
			.SetConsumerStage(vk::PipelineStageFlagBits::eTopOfPipe)
			.Submit(*cmdBuf);

		Plat_DebugString("%s(): Initializing \"%s\" to %s\n", __FUNCTION__, m_DebugName.c_str(), vk::to_string(m_DefaultLayout).c_str());
		cmdBuf->Submit();
	}

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
	case PSF::eTopOfPipe:
	case PSF::eBottomOfPipe:
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

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetAspects(vk::ImageAspectFlags aspects)
{
	m_Barrier.subresourceRange.aspectMask = aspects;
	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetAspectsFromFormat(vk::Format imgFormat)
{
	return SetAspects(FormatInfo::GetAspects(imgFormat));
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetFullSubresourceRange(uint32_t arrayLayerCount, uint32_t mipLevelCount)
{
	auto& srr = m_Barrier.subresourceRange;

	srr.baseArrayLayer = 0;
	srr.baseMipLevel = 0;
	srr.layerCount = arrayLayerCount;
	srr.levelCount = mipLevelCount;

	return *this;
}

ImageMemoryBarrierFactory& ImageMemoryBarrierFactory::SetFullSubresourceRange(const vk::ImageCreateInfo& ci)
{
	return SetFullSubresourceRange(ci.arrayLayers, ci.mipLevels);
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

		if (setAspects)
			SetAspectsFromFormat(ci.format);

		if (setFullSubresourceRange)
			SetFullSubresourceRange(ci.arrayLayers, ci.mipLevels);
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
	if (m_Barrier.newLayout != m_Barrier.oldLayout)
		cmdBuf.pipelineBarrier(m_SrcStage, m_DstStage, {}, {}, {}, m_Barrier);

	return *this;
}

DescriptorSetLayoutFactory& DescriptorSetLayoutFactory::AddBinding(uint32_t binding, vk::DescriptorType type,
	vk::ShaderStageFlagBits stageFlags, uint32_t arraySize)
{
	m_Bindings.emplace_back(binding, type, arraySize, stageFlags);
	return UpdateBindingsPointers();
}

vk::UniqueDescriptorSetLayout DescriptorSetLayoutFactory::Create() const
{
	assert(m_CreateInfo.pBindings == m_Bindings.data());
	assert(m_CreateInfo.bindingCount == m_Bindings.size());
	return g_ShaderDevice.GetVulkanDevice().createDescriptorSetLayoutUnique(
		m_CreateInfo, g_ShaderDeviceMgr.GetAllocationCallbacks());
}

DescriptorSetLayoutFactory& DescriptorSetLayoutFactory::UpdateBindingsPointers()
{
	m_CreateInfo.pBindings = m_Bindings.data();
	m_CreateInfo.bindingCount = m_Bindings.size();
	return *this;
}
