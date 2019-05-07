#pragma once

#include "TF2Vulkan/LogicalState.h"

#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishadershadow.h>

namespace TF2Vulkan
{
	class IVulkanCommandBuffer;

	class IStateManagerStatic : public IShaderShadow
	{
	public:
		virtual void ApplyState(LogicalShadowStateID id, IVulkanCommandBuffer& buf) = 0;
		virtual void ApplyCurrentState(IVulkanCommandBuffer& buf) = 0;

		virtual LogicalShadowStateID TakeSnapshot() = 0;
		virtual bool IsTranslucent(LogicalShadowStateID id) const = 0;
		virtual bool IsAlphaTested(LogicalShadowStateID id) const = 0;
		virtual bool UsesVertexAndPixelShaders(LogicalShadowStateID id) const = 0;
		virtual bool IsDepthWriteEnabled(LogicalShadowStateID id) const = 0;

		virtual void SetRenderTargetEx(int rtID, ShaderAPITextureHandle_t colTex, ShaderAPITextureHandle_t depthTex) = 0;

		virtual void SetState(LogicalShadowStateID id) = 0;
		virtual const LogicalShadowState& GetState(LogicalShadowStateID id) const = 0;

		// Helpers
		void SetState(StateSnapshot_t id)
		{
			return SetState(Util::SafeConvert<LogicalShadowStateID>(id));
		}
		bool IsTranslucent(StateSnapshot_t id) const
		{
			return IsTranslucent(Util::SafeConvert<LogicalShadowStateID>(id));
		}
		bool IsAlphaTested(StateSnapshot_t id) const
		{
			return IsAlphaTested(Util::SafeConvert<LogicalShadowStateID>(id));
		}
		bool UsesVertexAndPixelShaders(StateSnapshot_t id) const
		{
			return UsesVertexAndPixelShaders(Util::SafeConvert<LogicalShadowStateID>(id));
		}
		bool IsDepthWriteEnabled(StateSnapshot_t id) const
		{
			return IsDepthWriteEnabled(Util::SafeConvert<LogicalShadowStateID>(id));
		}
		const LogicalShadowState& GetState(StateSnapshot_t id) const
		{
			return GetState(Util::SafeConvert<LogicalShadowStateID>(id));
		}
	};

	extern IStateManagerStatic& g_StateManagerStatic;
}
