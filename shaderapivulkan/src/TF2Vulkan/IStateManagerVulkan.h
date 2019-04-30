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

		virtual void ApplyState(VulkanStateID stateID) = 0;

		VulkanStateID ApplyState(
			LogicalShadowStateID staticID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState)
		{
			ApplyState(FindOrCreateState(staticID, staticState, dynamicState));
		}
	};

	extern IStateManagerVulkan& g_StateManagerVulkan;
}
