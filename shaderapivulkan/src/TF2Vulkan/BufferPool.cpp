#include "BufferPool.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "VulkanFactories.h"

using namespace TF2Vulkan;

vma::AllocatedBuffer BufferPoolContiguous::CreateBackingBuffer(vk::BufferUsageFlags usage) const
{
	return Factories::BufferFactory{}
		.SetUsage(usage)
		.SetSize(m_BackingBufferSize)
		.SetMemoryRequiredFlags(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
		.SetAllowMapping()
		.Create();
}

static size_t GetAlignment(vk::BufferUsageFlags usage)
{
	const auto& limits = g_ShaderDeviceMgr.GetAdapterLimits();

	switch ((vk::BufferUsageFlagBits)(VkBufferUsageFlags)usage)
	{
	case vk::BufferUsageFlagBits::eUniformBuffer:
		return limits.minUniformBufferOffsetAlignment;
	case vk::BufferUsageFlagBits::eStorageBuffer:
		return limits.minStorageBufferOffsetAlignment;
	case vk::BufferUsageFlagBits::eStorageTexelBuffer:
		return limits.minTexelBufferOffsetAlignment;

	default:
		return 1;
	}
}

BufferPoolContiguous::BufferPoolContiguous(size_t backingBufferSize, vk::BufferUsageFlags usage) :
	m_BackingBufferSize(backingBufferSize),
	m_ElementAlignment(GetAlignment(usage)),
	m_BackingBuffer(CreateBackingBuffer(usage))
{
}

void BufferPoolContiguous::Update(const void* data, size_t size, size_t offset)
{
	if (offset % m_ElementAlignment)
		throw VulkanException("Invalid offset alignment", EXCEPTION_DATA());

	m_BackingBuffer.GetAllocation().Write(data, size, offset);
}

auto BufferPoolContiguous::GetBackingBufferInfo() const -> BufferInfo
{
	return BufferInfo{ m_BackingBuffer.GetBuffer(), m_BackingBufferSize, m_BackingBufferSize };
}

BufferPoolEntry BufferPoolContiguous::Create(size_t size)
{
	ASSERT_MAIN_THREAD();

	size = ALIGN_VALUE(size, m_ElementAlignment);

	auto offset = m_NextOffset;
	const auto maybeNextOffset = m_NextOffset + size;
	if (maybeNextOffset > m_BackingBufferSize)
	{
		offset = 0;
		m_NextOffset = m_ElementAlignment;
	}
	else
	{
		m_NextOffset = maybeNextOffset;
	}

	assert((offset + size) <= m_BackingBufferSize);
	assert(ALIGN_VALUE(offset, m_ElementAlignment) == offset);

	return BufferPoolEntry(size, offset, *this);
}

BufferPool::BufferPool(vk::BufferUsageFlags usage)
{
	//VmaPoolCreateInfo ci{};
	//ci.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
	//ci.memoryTypeIndex =
}

auto BufferPool::GetBufferInfo(size_t offset) const -> BufferInfo
{
	NOT_IMPLEMENTED_FUNC();
}

BufferPoolEntry BufferPool::Create(size_t size)
{
	NOT_IMPLEMENTED_FUNC();
}

void BufferPool::Update(const void* data, size_t size, size_t offset)
{
	NOT_IMPLEMENTED_FUNC();
}
