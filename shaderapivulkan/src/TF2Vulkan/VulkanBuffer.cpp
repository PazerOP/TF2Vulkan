#include "VulkanBuffer.h"

using namespace TF2Vulkan;

struct VulkanBuffer::AllocatedBuffer : IBufferBase
{
	const vk::Buffer& GetBuffer() const override { return m_Buffer.GetBuffer(); }

	vma::AllocatedBuffer m_Buffer;
};
