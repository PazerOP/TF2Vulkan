#include "interface/internal/IShaderDeviceInternal.h"
#include "VulkanFactories.h"

using namespace TF2Vulkan;
using namespace TF2Vulkan::Factories;

static const auto MAP_REQUIRED_FLAGS = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
static const auto& MAP_REQUIRED_FLAGS_VK = *reinterpret_cast<const VkMemoryPropertyFlags*>(&MAP_REQUIRED_FLAGS);

template<typename T>
T& FactoryBase<T>::SetDebugName(std::string&& dbgName)
{
	m_DebugName = std::move(dbgName);
	return *static_cast<T*>(this);
}

template<typename T>
T& FactoryBase<T>::SetDebugName(const std::string_view& dbgName)
{
	return SetDebugName(std::string(dbgName));
}

template struct FactoryBase<BufferFactory>;
template struct FactoryBase<ImageFactory>;

BufferFactory& BufferFactory::SetAllowMapping(bool allow)
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

	return *this;
}

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

ImageFactory& ImageFactory::SetMemoryUsage(VmaMemoryUsage usage)
{
	m_AllocInfo.usage = usage;
	return *this;
}

ImageFactory& ImageFactory::SetCreateInfo(const vk::ImageCreateInfo& createInfo)
{
	m_CreateInfo = createInfo;
	return *this;
}

vma::AllocatedImage ImageFactory::Create() const
{
	auto created = g_ShaderDevice.GetVulkanAllocator().createImageUnique(m_CreateInfo, m_AllocInfo);

	if (!m_DebugName.empty())
		g_ShaderDevice.SetDebugName(created.GetImage(), m_DebugName.c_str());

	return created;
}
