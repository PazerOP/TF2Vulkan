#include "vk_mem_alloc.hpp"

using namespace vma;

UniqueAllocator vma::createAllocatorUnique(const AllocatorCreateInfo& createInfo)
{
	VmaAllocator retVal;
	if (auto result = vk::Result(vmaCreateAllocator(&createInfo, &retVal));
		result != vk::Result::eSuccess)
	{
		Warning("[TF2Vulkan] %s(): %s\n", __FUNCTION__, vk::to_string(result).c_str());
		assert(false);
		return nullptr;
	}

	return UniqueAllocator(retVal);
}

UniqueAllocation::UniqueAllocation(VmaAllocator allocator, VmaAllocation allocation) :
	m_Allocator(allocator), m_Allocation(allocation)
{
}

UniqueAllocation::UniqueAllocation(UniqueAllocation&& other) noexcept
{
	m_Allocator = other.m_Allocator;
	m_Allocation = other.m_Allocation;
	other.m_Allocator = nullptr;
	other.m_Allocation = nullptr;
}

UniqueAllocation& UniqueAllocation::operator=(UniqueAllocation&& other) noexcept
{
	m_Allocator = other.m_Allocator;
	m_Allocation = other.m_Allocation;
	other.m_Allocator = nullptr;
	other.m_Allocation = nullptr;
	return *this;
}

UniqueAllocation::~UniqueAllocation()
{
	assert(!m_Allocator == !m_Allocation);
	if (m_Allocator)
	{
		vmaFreeMemory(m_Allocator, m_Allocation);
		m_Allocator = nullptr;
		m_Allocation = nullptr;
	}
}

AllocationInfo UniqueAllocation::getAllocationInfo() const
{
	AllocationInfo retVal;
	vmaGetAllocationInfo(m_Allocator, m_Allocation, &retVal);
	return retVal;
}

UniqueAllocator::UniqueAllocator(VmaAllocator allocator) :
	m_Allocator(allocator)
{
}

AllocatedBuffer UniqueAllocator::createBufferUnique(const vk::BufferCreateInfo& bufCreateInfo,
	const AllocationCreateInfo& allocCreateInfo)
{
	auto cBufCreateInfo = (VkBufferCreateInfo)bufCreateInfo;

	VkBuffer outBuf;
	VmaAllocation outAllocation;
	if (auto result = vk::Result(vmaCreateBuffer(m_Allocator.get(), &cBufCreateInfo,
		&allocCreateInfo, &outBuf, &outAllocation, nullptr));
		result != vk::Result::eSuccess)
	{
		Warning("[TF2Vulkan] %s(): %s\n", __FUNCTION__, vk::to_string(result).c_str());
		assert(false);
		return {};
	}

	return { outBuf, UniqueAllocation(m_Allocator.get(), outAllocation) };
}

AllocatedImage UniqueAllocator::createImageUnique(const vk::ImageCreateInfo& imgCreateInfo,
	const AllocationCreateInfo& allocCreateInfo)
{
	auto cImgCreateInfo = (VkImageCreateInfo)imgCreateInfo;

	VkImage outImg;
	VmaAllocation outAllocation;
	if (auto result = vk::Result(vmaCreateImage(m_Allocator.get(), &cImgCreateInfo,
		&allocCreateInfo, &outImg, &outAllocation, nullptr));
		result != vk::Result::eSuccess)
	{
		Warning("[TF2Vulkan] %s(): %s\n", __FUNCTION__, vk::to_string(result).c_str());
		assert(false);
		return {};
	}

	return { outImg, UniqueAllocation(m_Allocator.get(), outAllocation) };
}

AllocatedBuffer::AllocatedBuffer(VkBuffer buf, UniqueAllocation&& allocation) :
	m_Buffer(vk::Buffer(buf)), m_Allocation(std::move(allocation))
{
}

AllocatedImage::AllocatedImage(VkImage img, UniqueAllocation&& allocation) :
	m_Image(vk::Image(img)), m_Allocation(std::move(allocation))
{
}

void detail::Deleter::operator()(VmaAllocator allocator) const
{
	if (allocator)
		vmaDestroyAllocator(allocator);
}
