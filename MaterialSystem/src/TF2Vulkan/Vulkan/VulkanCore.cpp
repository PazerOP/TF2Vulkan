#include "VulkanCore.h"

using namespace TF2Vulkan;

VulkanCore::VulkanCore()
{
	vk::ApplicationInfo info("TF2Vulkan", 1, "Source/TF2Vulkan", 1, VK_API_VERSION_1_1);
}
