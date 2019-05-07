#pragma once

namespace TF2Vulkan
{
	class IVulkanCommandBuffer;

	class PixScope final
	{
	public:
		PixScope(IVulkanCommandBuffer& buf);
		~PixScope();

	private:
		IVulkanCommandBuffer& m_CmdBuf;
	};
}
