#pragma once

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace vma
{
	namespace detail
	{
		struct Deleter
		{
			void operator()(VmaAllocator allocator) const;
		};
	}

	struct AllocatorCreateInfo : public VmaAllocatorCreateInfo
	{
		constexpr AllocatorCreateInfo() :
			VmaAllocatorCreateInfo{}
		{
		}
	};

	struct AllocationCreateInfo : public VmaAllocationCreateInfo
	{
		constexpr AllocationCreateInfo() :
			VmaAllocationCreateInfo{}
		{
		}
	};

	struct AllocationInfo : public VmaAllocationInfo
	{
		constexpr AllocationInfo() :
			VmaAllocationInfo{}
		{
		}
	};

	class UniqueAllocation final
	{
	public:
		UniqueAllocation() = default;
		UniqueAllocation(VmaAllocator allocator, VmaAllocation allocation);
		UniqueAllocation(const UniqueAllocation&) = delete;
		UniqueAllocation& operator=(const UniqueAllocation&) = delete;
		UniqueAllocation(UniqueAllocation&& other) noexcept;
		UniqueAllocation& operator=(UniqueAllocation&& other) noexcept;
		~UniqueAllocation();

		AllocationInfo getAllocationInfo() const;

	private:
		VmaAllocator m_Allocator;
		VmaAllocation m_Allocation;
	};

	struct AllocatedBuffer final
	{
		AllocatedBuffer() = default;
		AllocatedBuffer(VkBuffer buf, UniqueAllocation&& allocation);

		vk::UniqueBuffer m_Buffer;
		UniqueAllocation m_Allocation;
	};

	struct AllocatedImage final
	{
		AllocatedImage() = default;
		AllocatedImage(VkImage img, UniqueAllocation&& allocation);

		vk::UniqueImage m_Image;
		UniqueAllocation m_Allocation;
	};

	class UniqueAllocator final
	{
	public:
		UniqueAllocator(VmaAllocator allocator = nullptr);

		AllocatedBuffer createBufferUnique(const vk::BufferCreateInfo& bufCreateInfo,
			const AllocationCreateInfo& allocCreateInfo);
		AllocatedImage createImageUnique(const vk::ImageCreateInfo& imgCreateInfo,
			const AllocationCreateInfo& allocCreateInfo);

		operator bool() const { return !!m_Allocator; }

	private:
		std::unique_ptr<std::remove_pointer_t<VmaAllocator>, detail::Deleter> m_Allocator;
	};

	UniqueAllocator createAllocatorUnique(const AllocatorCreateInfo& createInfo);
}
