#pragma once

#include <variant>

namespace TF2Vulkan
{
	class VulkanBuffer
	{
	public:

	private:
		struct AllocatedBuffer;
		struct ExistingBuffer;
		struct IBufferBase
		{
			virtual ~IBufferBase() = default;

			virtual const vk::Buffer& GetBuffer() const = 0;
		};

		std::unique_ptr<IBufferBase> m_Buffer;
	};
}
