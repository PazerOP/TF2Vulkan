#pragma once

#include <TF2Vulkan/Util/Checked.h>
#include <TF2Vulkan/Util/Enums.h>
#include <TF2Vulkan/Util/UniqueObject.h>

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace vma
{
	enum class MemoryType
	{
		eUnknown = VMA_MEMORY_USAGE_UNKNOWN,
		eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
		eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
		eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
		eGpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU,
	};

	enum class AllocationCreateFlagBits
	{
		eDedicatedMemory = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		eNeverAllocate = VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,
		eCreateMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		eCanBecomeLost = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT,
		eCanMakeOtherLost = VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT,
		eUserDataCopyString = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
		eUpperAddress = VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
		eBestFit = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT,
		eWorstFit = VMA_ALLOCATION_CREATE_STRATEGY_WORST_FIT_BIT,
		eFirstFit = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT,
	};
}

ENABLE_ENUM_FLAG_OPS(vma::MemoryType);
ENABLE_ENUM_FLAG_OPS(vma::AllocationCreateFlagBits);

namespace vma
{
	namespace detail
	{
		struct Deleter
		{
			void operator()(VmaAllocator allocator) const;
		};

		struct AllocationDeleter
		{
			AllocationDeleter(VmaAllocator allocator = nullptr) : m_Allocator(allocator) {}
			void operator()(VmaAllocation allocation) const noexcept;
			VmaAllocator m_Allocator;
		};
	}

	struct AllocationInfo : public VmaAllocationInfo
	{
		constexpr AllocationInfo() :
			VmaAllocationInfo{}
		{
		}
	};

	namespace detail
	{
		struct AllocatedObjectDeleter;
	}

	class UniqueAllocation final
	{
	public:
		UniqueAllocation() = default;
		UniqueAllocation(VmaAllocator allocator, VmaAllocation allocation);

		AllocationInfo getAllocationInfo() const;

		bool IsMapped() const;

		void Write(const void* srcData, size_t srcSize, size_t dstOffset = 0);
		void Read(void* dstData, size_t srcSize) const;

		template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
		void Write(const T& data, size_t dstOffset = 0)
		{
			return Write(&data, sizeof(data), dstOffset);
		}

		// ONLY USE THESE IF YOU REALLY NEED TO DIRECTLY WRITE INTO THE BUFFER.
		// OTHERWISE, *PLEASE* JUST USE READ/WRITE ABOVE FOR SAFER ACCESS.
		std::byte* data();
		const std::byte* data() const;

		operator bool() const;

		VmaAllocation GetAllocation() const;
		VmaAllocator GetAllocator() const;

	private:
		friend class MappedMemory;
		friend struct detail::AllocatedObjectDeleter;

		std::unique_ptr<std::remove_pointer_t<VmaAllocation>, detail::AllocationDeleter> m_Allocation;
	};

	namespace detail
	{
		struct AllocatedObjectDeleter
		{
			AllocatedObjectDeleter() = default;
			AllocatedObjectDeleter(UniqueAllocation&& allocation);
			void operator()(vk::Buffer& buffer) noexcept;
			void operator()(vk::Image& image) noexcept;
			UniqueAllocation m_Allocation;
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

	struct AllocatedBuffer final
	{
		AllocatedBuffer() = default;
		AllocatedBuffer(VkBuffer buf, UniqueAllocation&& allocation, size_t realSize);

		const vk::Buffer& GetBuffer() const;
		UniqueAllocation& GetAllocation();
		const UniqueAllocation& GetAllocation() const;

		size_t size() const;

		operator bool() const;

	private:
		size_t m_RealSize = 0;
		Util::UniqueObject<vk::Buffer, detail::AllocatedObjectDeleter> m_Buffer;
	};

	struct AllocatedImage final
	{
		AllocatedImage() = default;
		AllocatedImage(VkImage img, UniqueAllocation&& allocation);

		const vk::Image& GetImage() const;
		UniqueAllocation& GetAllocation();
		const UniqueAllocation& GetAllocation() const;

		operator bool() const;

	private:
		Util::UniqueObject<vk::Image, detail::AllocatedObjectDeleter> m_Image;
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
