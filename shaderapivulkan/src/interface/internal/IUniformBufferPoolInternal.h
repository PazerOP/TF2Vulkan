#pragma once

#include <TF2Vulkan/IUniformBufferPool.h>

namespace TF2Vulkan
{
	class IUniformBufferPoolInternal : public IUniformBufferPool
	{
	public:
		virtual const vk::Buffer& GetBackingBuffer() const = 0;
	};
}
