#include "GraphicsPipeline.h"
#include "ShaderDevice.h"
#include "ShadowStateManager.h"
#include "TF2Vulkan/shaders/VulkanShaderManager.h"

#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_utility.h>

using namespace TF2Vulkan;

namespace
{
	class VulkanShaderShadow final : public TF2Vulkan::ShadowStateManager
	{
	public:
	};
}

static VulkanShaderShadow s_VSS;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VulkanShaderShadow, IShaderShadow, SHADERSHADOW_INTERFACE_VERSION, s_VSS);

ShadowStateManager& TF2Vulkan::g_ShadowStateManager = s_VSS;
