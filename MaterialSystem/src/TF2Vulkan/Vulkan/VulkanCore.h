#pragma once

#include <vulkan/vulkan.hpp>

namespace TF2Vulkan
{
	class VulkanCore final
	{
	public:
		VulkanCore();

	private:
		vk::Instance m_Instance;
	};
}
