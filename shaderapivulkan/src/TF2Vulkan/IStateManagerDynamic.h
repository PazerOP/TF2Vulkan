#pragma once

#include "LogicalState.h"
#include "interface/internal/IShaderAPIInternal.h"

#include <TF2Vulkan/Util/InPlaceVector.h>

#include <Color.h>
#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishaderdynamic.h>

namespace TF2Vulkan
{
	class IShaderAPI_StateManagerDynamic : public IShaderAPIInternal
	{
	public:
		void ApplyState() const;

		void ClearColor3ub(uint8_t r, uint8_t g, uint8_t b) override final;
		void ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override final;

		void SetViewports(int count, const ShaderViewport_t* viewports) override final;

		void SetAnisotropicLevel(int anisoLevel) override final;
		void SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex) override final;
		void SetDefaultState() override final;
		int GetCurrentNumBones() const override final;
		void Bind(IMaterial* material) override final;

		// Helpers
		void SetOverbright(float overbright);

		const LogicalDynamicState& GetDynamicState() const { return m_State; }

	private:
		LogicalDynamicState m_State;
		bool m_Dirty = true;
	};

	extern IShaderAPI_StateManagerDynamic& g_StateManagerDynamic;
}
