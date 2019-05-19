#pragma once

#include <TF2Vulkan/IBufferPool.h>

namespace TF2Vulkan
{
	class IBufferPoolInternal : public IBufferPool
	{
	public:
		struct BufferInfo
		{
			BufferInfo(vk::Buffer buffer, size_t size, size_t localSize) :
				m_Buffer(buffer), m_Size(size), m_LocalSize(localSize)
			{
			}

			vk::Buffer m_Buffer;
			size_t m_Size;
			size_t m_LocalSize;
		};

		virtual BufferInfo GetBufferInfo(size_t offset) const = 0;
		vk::Buffer GetBuffer(size_t offset) const { return GetBufferInfo(offset).m_Buffer; }

		BufferInfo GetBufferInfo(const BufferPoolEntry& buf) const
		{
			assert(&buf.GetPool() == this);
			return GetBufferInfo(buf.GetOffset());
		}
	};

	class IBufferPoolContiguous : public IBufferPoolInternal
	{
	public:
		virtual BufferInfo GetBackingBufferInfo() const = 0;

		BufferInfo GetBufferInfo(size_t offset) const override final
		{
			const auto backingInfo = GetBackingBufferInfo();
			return BufferInfo{ backingInfo.m_Buffer, backingInfo.m_Size, backingInfo.m_Size - offset };
		}
	};
}
