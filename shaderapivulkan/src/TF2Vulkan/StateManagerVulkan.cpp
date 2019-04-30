#include "IStateManagerVulkan.h"
#include "LogicalState.h"
#include "ShaderDevice.h"
#include "shaders/VulkanShaderManager.h"

#include <unordered_map>

using namespace TF2Vulkan;

namespace
{
	struct VulkanStateKey
	{
		constexpr VulkanStateKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);

		DEFAULT_WEAK_EQUALITY_OPERATOR(VulkanStateKey);

		CUtlSymbolDbg m_VSName;
		int m_VSStaticIndex;

		CUtlSymbolDbg m_PSName;
		int m_PSStaticIndex;
	};

	struct Pipeline
	{
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStageCIs;

		std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
		std::vector<vk::VertexInputBindingDescription> m_VertexInputBindingDescriptions;
		vk::PipelineVertexInputStateCreateInfo m_VertexInputStateCI;

		vk::PipelineInputAssemblyStateCreateInfo m_InputAssemblyStateCI;

		vk::GraphicsPipelineCreateInfo m_CreateInfo;
		vk::UniquePipeline m_Pipeline;

		bool operator!() const { return !m_Pipeline; }
		VulkanStateID m_ID;
	};
}

STD_HASH_DEFINITION(VulkanStateKey,
	v.m_VSName,
	v.m_VSStaticIndex,

	v.m_PSName,
	v.m_PSStaticIndex
);

namespace
{
	class StateManagerVulkan final : public IStateManagerVulkan
	{
	public:
		void ApplyState(VulkanStateID stateID) override { NOT_IMPLEMENTED_FUNC(); }

		VulkanStateID FindOrCreateState(
			LogicalShadowStateID staticID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState);

	private:
		Pipeline CreatePipeline(const VulkanStateKey& key) const;

		vk::PipelineCache m_PipelineCache;

		std::unordered_map<VulkanStateKey, Pipeline> m_StatesToPipelines;
		std::vector<const Pipeline*> m_IDsToPipelines;
	};
}

static StateManagerVulkan s_SMVulkan;
IStateManagerVulkan& TF2Vulkan::g_StateManagerVulkan = s_SMVulkan;

static vk::PipelineShaderStageCreateInfo CreateStageInfo(
	const CUtlSymbolDbg& name, int staticIndex)
{
	vk::PipelineShaderStageCreateInfo retVal;

	retVal.module = g_ShaderManager.FindOrCreateShader(
		name, staticIndex);

	retVal.pName = name.String();

	return retVal;
}

Pipeline StateManagerVulkan::CreatePipeline(const VulkanStateKey& key) const
{
	Pipeline retVal;

	// Shader stage create info(s)
	{
		auto& cis = retVal.m_ShaderStageCIs;
		cis.emplace_back(CreateStageInfo(key.m_VSName.String(), key.m_VSStaticIndex));
		cis.emplace_back(CreateStageInfo(key.m_PSName.String(), key.m_PSStaticIndex));
	}

	// Vertex input state create info
	{
		using VIAD = vk::VertexInputAttributeDescription;
		auto& attrs = retVal.m_VertexInputAttributeDescriptions;
		attrs.emplace_back(VIAD()
		NOT_IMPLEMENTED_FUNC();

		auto& binds = retVal.m_VertexInputBindingDescriptions;
		NOT_IMPLEMENTED_FUNC();

		auto& ci = retVal.m_VertexInputStateCI;
		ci.pVertexBindingDescriptions = binds.data();
		Util::SafeConvert(binds.size(), ci.vertexBindingDescriptionCount);

		ci.pVertexAttributeDescriptions = attrs.data();
		Util::SafeConvert(attrs.size(), ci.vertexAttributeDescriptionCount);
	}

	// Graphics pipeline
	{
		vk::GraphicsPipelineCreateInfo& ci = retVal.m_CreateInfo;

		Util::SafeConvert(retVal.m_ShaderStageCIs.size(), ci.stageCount);
		ci.pStages = retVal.m_ShaderStageCIs.data();

		retVal.m_Pipeline = g_ShaderDevice.GetVulkanDevice().createGraphicsPipelineUnique(m_PipelineCache, ci);
	}

	return retVal;
}

VulkanStateID StateManagerVulkan::FindOrCreateState(LogicalShadowStateID staticID,
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState)
{
	const VulkanStateKey key(staticState, dynamicState);
	auto& pl = m_StatesToPipelines[key];
	if (!pl)
	{
		pl = CreatePipeline(key);
		m_IDsToPipelines.push_back(&pl);
		assert(VulkanStateID(m_IDsToPipelines.size() - 1) == pl.m_ID);
	}

	return pl.m_ID;
}

constexpr VulkanStateKey::VulkanStateKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_VSName(staticState.m_VSName),
	m_VSStaticIndex(staticState.m_VSStaticIndex),

	m_PSName(staticState.m_PSName),
	m_PSStaticIndex(staticState.m_PSStaticIndex)
{
}
