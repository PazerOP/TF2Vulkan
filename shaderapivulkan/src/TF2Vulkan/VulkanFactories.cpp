#include "ShaderDevice.h"
#include "VulkanFactories.h"

using namespace TF2Vulkan;
using namespace TF2Vulkan::Factories;

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

BufferFactory& BufferFactory::SetUsage(const vk::BufferUsageFlags& usage)
{
	m_Usage = usage;
	return *this;
}

BufferFactory& BufferFactory::SetSize(size_t size)
{
	m_Size = size;
	return *this;
}

BufferFactory& BufferFactory::SetInitialData(const void* initialData, size_t initialDataSize,
	size_t writeOffset)
{
	m_InitialData = initialData;
	m_InitialDataSize = initialDataSize;
	m_InitialDataWriteOffset = writeOffset;

	if (m_Size == 0)
		m_Size = m_InitialDataSize + m_InitialDataWriteOffset;

	return *this;
}

BufferFactory& BufferFactory::SetMemoryRequiredFlags(const vk::MemoryPropertyFlags& flags)
{
	m_MemoryRequiredFlags = flags;
	return *this;
}

BufferFactory& BufferFactory::SetDebugName(std::string&& dbgName)
{
	m_DebugName = std::move(dbgName);
	return *this;
}

vma::AllocatedBuffer BufferFactory::Create() const
{
	vk::BufferCreateInfo bufCI;
	bufCI.usage = m_Usage;
	bufCI.size = m_Size;

	vma::AllocationCreateInfo allocCI;
	allocCI.requiredFlags = (VkMemoryPropertyFlags)m_MemoryRequiredFlags;

	const bool hasInitialData = m_InitialData && m_InitialDataSize > 0;

	const auto mapRequiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	const auto mapRequiredFlagsVk = VkMemoryPropertyFlags(mapRequiredFlags);

	// TODO: Create a staging buffer etc right here
	if (hasInitialData)
		allocCI.requiredFlags |= mapRequiredFlagsVk;

	auto created = g_ShaderDevice.GetVulkanAllocator().createBufferUnique(bufCI, allocCI);

	if (!m_DebugName.empty())
		g_ShaderDevice.SetDebugName(created.GetBuffer(), m_DebugName.c_str());

	if (hasInitialData)
	{
		auto mapped = created.GetAllocation().map();
		mapped.Write(m_InitialData, m_InitialDataSize, m_InitialDataWriteOffset);
	}

	return created;
}
