#pragma once

#include "interface/internal/IBufferPoolInternal.h"

namespace TF2Vulkan
{
	// A buffer pool that just operates on offsets of a single large base buffer.
	class BufferPoolContiguous final : public IBufferPoolContiguous
	{
	public:
		BufferPoolContiguous(size_t backingBufferSize, vk::BufferUsageFlags usage);

		BufferInfo GetBackingBufferInfo() const override;
		BufferPoolEntry Create(size_t size) override;
		void Update(const void* data, size_t size, size_t offset) override;

		BufferInfo GetBufferInfo(size_t offset) const override;

	private:
		size_t m_ElementAlignment;
		size_t m_BackingBufferSize;
		size_t m_NextOffset = 0;

		struct BufferSliceInfo
		{
			size_t m_Length = 0;
		};
		std::vector<BufferSliceInfo> m_SliceInfo;

		vma::AllocatedBuffer CreateBackingBuffer(vk::BufferUsageFlags usage) const;
		vma::AllocatedBuffer m_BackingBuffer;
	};

	class BufferPool final : public IBufferPoolInternal
	{
	public:
		BufferPool(vk::BufferUsageFlags usage);

		BufferInfo GetBufferInfo(size_t offset) const override;
		BufferPoolEntry Create(size_t size) override;
		void Update(const void* data, size_t size, size_t offset) override;

	private:
		struct VmaPoolDeleter
		{
			VmaAllocator m_Allocator;
			void operator()(VmaPool pool) const
			{
				vmaDestroyPool(m_Allocator, pool);
			}
		};
		std::unique_ptr<std::remove_pointer_t<VmaPool>, VmaPoolDeleter> m_Pool;
	};
}
