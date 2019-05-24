#pragma once

namespace TF2Vulkan
{
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
			const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState,
			const IMesh& mesh) = 0;

		virtual void ApplyState(VulkanStateID stateID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState, const IMesh& mesh, IVulkanCommandBuffer& buf) = 0;

		VulkanStateID ApplyState(const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState, const IMesh& mesh,
			IVulkanCommandBuffer& buf)
		{
			const auto& stateID = FindOrCreateState(staticState, dynamicState, mesh);
			ApplyState(stateID, staticState, dynamicState, mesh, buf);
			return stateID;
		}
	};

	extern IStateManagerVulkan& g_StateManagerVulkan;
}
