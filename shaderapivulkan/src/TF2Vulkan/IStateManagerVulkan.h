#pragma once

namespace TF2Vulkan
{
	enum class LogicalShadowStateID : size_t;
	struct LogicalShadowState;
	struct LogicalDynamicState;

	enum class VulkanStateID : size_t
	{
		Invalid = size_t(-1),
	};

	class IStateManagerVulkan
	{
	protected:
		virtual ~IStateManagerVulkan() = default;

	public:
		virtual VulkanStateID FindOrCreateState(
			LogicalShadowStateID staticID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState) = 0;

		virtual void ApplyState(VulkanStateID stateID, const vk::CommandBuffer& buf) = 0;

		VulkanStateID ApplyState(
			LogicalShadowStateID staticID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState, const vk::CommandBuffer& buf)
		{
			const auto& stateID = FindOrCreateState(staticID, staticState, dynamicState);
			ApplyState(stateID, buf);
			return stateID;
		}
	};

	extern IStateManagerVulkan& g_StateManagerVulkan;
}
