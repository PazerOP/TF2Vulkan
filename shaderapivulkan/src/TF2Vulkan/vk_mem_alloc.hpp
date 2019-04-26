#pragma once

#include <TF2Vulkan/Util/Checked.h>
#include <TF2Vulkan/Util/ClassPrefabs.h>

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

	class UniqueAllocation;

	class MappedMemory final : Util::DisableCopy
	{
	public:
		void Write(const void* srcData, size_t srcSize, size_t dstOffset = 0);
		void Read(void* dstData, size_t srcSize) const;

	private:
		MappedMemory(UniqueAllocation* allocation);
		struct Unmapper
		{
			void operator()(UniqueAllocation* alloc) const;
		};

		std::unique_ptr<UniqueAllocation, Unmapper> m_Allocation;
		friend class UniqueAllocation;
	};

	class UniqueAllocation final : Util::DisableCopy
	{
	public:
		UniqueAllocation() = default;
		UniqueAllocation(VmaAllocator allocator, VmaAllocation allocation);
		UniqueAllocation(UniqueAllocation&& other) noexcept;
		UniqueAllocation& operator=(UniqueAllocation&& other) noexcept;
		~UniqueAllocation();

		AllocationInfo getAllocationInfo() const;

		[[nodiscard]] MappedMemory map();

	private:
		friend class MappedMemory;
		void* m_MappedData = nullptr;

		VmaAllocator m_Allocator;
		VmaAllocation m_Allocation;
	};

	struct AllocatedBuffer final
	{
		AllocatedBuffer() = default;
		AllocatedBuffer(VkBuffer buf, UniqueAllocation&& allocation);

		vk::Buffer m_Buffer;
		UniqueAllocation m_Allocation;
	};

	struct AllocatedImage final
	{
		AllocatedImage() = default;
		AllocatedImage(VkImage img, UniqueAllocation&& allocation);

		vk::Image m_Image;
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
