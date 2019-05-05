#pragma once
#include "interface/internal/IVulkanCommandBuffer.h"

namespace TF2Vulkan
{
	class VulkanCommandBufferBase : public IVulkanCommandBuffer
	{
	public:
		void AddResource(vk::UniqueBuffer&& buffer) override final;

	private:
		struct INode
		{
			virtual ~INode() = default;
			std::unique_ptr<INode> m_Next;
		};
		struct BufferNode;

		std::unique_ptr<INode> m_FirstNode;
	};
}
