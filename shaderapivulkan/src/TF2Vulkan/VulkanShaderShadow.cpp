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

	protected:
		vk::UniquePipeline CreatePipeline(const ShadowState& state) const override;

	private:
		static std::vector<vk::PipelineShaderStageCreateInfo> GetStageInfo(const ShadowState& state);

		vk::PipelineCache m_PipelineCache;
	};
}

static VulkanShaderShadow s_VSS;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VulkanShaderShadow, IShaderShadow, SHADERSHADOW_INTERFACE_VERSION, s_VSS);

ShadowStateManager& TF2Vulkan::g_ShadowStateManager = s_VSS;

std::vector<vk::PipelineShaderStageCreateInfo> VulkanShaderShadow::GetStageInfo(const ShadowState& state)
{
	std::vector<vk::PipelineShaderStageCreateInfo> retVal;

	auto& stage = retVal.emplace_back();
	stage.module = g_ShaderManager.FindOrCreateShader(
		state.m_VertexShader.m_Name, state.m_VertexShader.m_StaticIndex);

	stage.pName = state.m_VertexShader.m_Name.String();

	return retVal;
}

vk::UniquePipeline VulkanShaderShadow::CreatePipeline(const ShadowState& state) const
{
	vk::GraphicsPipelineCreateInfo ci;

	const auto stages = GetStageInfo(state);
	Util::SafeConvert(stages.size(), ci.stageCount);
	ci.pStages = stages.data();

	return g_ShaderDevice.GetVulkanDevice().createGraphicsPipelineUnique(m_PipelineCache, ci);
}
