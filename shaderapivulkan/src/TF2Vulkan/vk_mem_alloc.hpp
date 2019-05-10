#pragma once

#include <TF2Vulkan/Util/Checked.h>
#include <TF2Vulkan/Util/UniqueObject.h>

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

		void Write(const void* srcData, size_t srcSize, size_t dstOffset = 0);
		void Read(void* dstData, size_t srcSize) const;

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
		AllocatedBuffer(VkBuffer buf, UniqueAllocation&& allocation);

		const vk::Buffer& GetBuffer() const;
		UniqueAllocation& GetAllocation();
		const UniqueAllocation& GetAllocation() const;

	private:
		Util::UniqueObject<vk::Buffer, detail::AllocatedObjectDeleter> m_Buffer;
	};

	struct AllocatedImage final
	{
		AllocatedImage() = default;
		AllocatedImage(VkImage img, UniqueAllocation&& allocation);

		const vk::Image& GetImage() const;
		UniqueAllocation& GetAllocation();
		const UniqueAllocation& GetAllocation() const;

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
