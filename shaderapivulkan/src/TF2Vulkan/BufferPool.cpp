#include "BufferPool.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "VulkanFactories.h"

#undef min
#undef max

using namespace TF2Vulkan;

vma::AllocatedBuffer BufferPoolContiguous::CreateBackingBuffer(vk::BufferUsageFlags usage) const
{
	return Factories::BufferFactory{}
		.SetUsage(usage)
		.SetSize(m_BackingBufferSize)
		.SetMemoryRequiredFlags(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
		.SetAllowMapping()
		.SetDebugName("BufferPoolContiguous "s + vk::to_string(usage))
		.Create();
}

static size_t GetAlignment(vk::BufferUsageFlags usage)
{
	const auto& limits = g_ShaderDeviceMgr.GetAdapterLimits();

	vk::DeviceSize alignment = 1;
	if (usage & vk::BufferUsageFlagBits::eUniformBuffer)
		alignment = std::max(alignment, limits.minUniformBufferOffsetAlignment);
	if (usage & vk::BufferUsageFlagBits::eStorageBuffer)
		alignment = std::max(alignment, limits.minStorageBufferOffsetAlignment);
	if (usage & vk::BufferUsageFlagBits::eStorageTexelBuffer)
		alignment = std::max(alignment, limits.minTexelBufferOffsetAlignment);
	if (usage & vk::BufferUsageFlagBits::eVertexBuffer)
		alignment = std::max<vk::DeviceSize>(alignment, 4);
	if (usage & vk::BufferUsageFlagBits::eIndexBuffer)
		alignment = std::max<vk::DeviceSize>(alignment, 2);

	return Util::SafeConvert<size_t>(alignment);
}

BufferPoolContiguous::BufferPoolContiguous(size_t backingBufferSize, vk::BufferUsageFlags usage) :
	m_BackingBufferSize(backingBufferSize),
	m_ElementAlignment(GetAlignment(usage)),
	m_BackingBuffer(CreateBackingBuffer(usage)),
	m_SliceInfo(backingBufferSize / m_ElementAlignment),
	m_Mutex(std::make_unique<Util::MutexDbg>())
{
}

void BufferPoolContiguous::Update(const void* data, size_t size, size_t offset)
{
	if (offset % m_ElementAlignment)
		throw VulkanException("Invalid offset alignment", EXCEPTION_DATA());

	m_BackingBuffer.GetAllocation().Write(data, size, offset);
}

std::byte* BufferPoolContiguous::GetBufferData(size_t offset)
{
	auto mapped = m_BackingBuffer.GetAllocation().data();
	return mapped ? (mapped + offset) : nullptr;
}

auto BufferPoolContiguous::GetBackingBufferInfo() const -> BufferInfo
{
	return BufferInfo{ m_BackingBuffer.GetBuffer(), m_BackingBufferSize };
}

BufferPoolEntry BufferPoolContiguous::Create(size_t size)
{
	std::lock_guard lock(*m_Mutex);
	LOG_FUNC_ANYTHREAD();

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
	assert((offset % m_ElementAlignment) == 0);

	m_SliceInfo.at(offset / m_ElementAlignment).m_Length = size;

	return BufferPoolEntry(size, offset, *this);
}

BufferPool::BufferPool(vk::BufferUsageFlags usage)
{
	NOT_IMPLEMENTED_FUNC();
	VmaPoolCreateInfo ci{};
	//ci.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;

	//vmaFindMemoryTypeIndex()
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

auto BufferPoolContiguous::GetBufferInfo(size_t offset) const -> BufferInfo
{
	assert((offset % m_ElementAlignment) == 0);

	const auto elemIdx = offset / m_ElementAlignment;

	return BufferInfo(m_BackingBuffer.GetBuffer(), m_SliceInfo.at(elemIdx).m_Length);
}
