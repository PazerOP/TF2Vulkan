#pragma once

#include "interface/internal/IShaderAPIInternal.h"

#include <TF2Vulkan/Util/InPlaceVector.h>

#include <Color.h>
#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishaderdynamic.h>

namespace TF2Vulkan
{
	class IStateManagerDynamic : public IShaderAPIInternal
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

		struct StdVtxShaderConstants
		{
			constexpr StdVtxShaderConstants() = default;

			float m_Overbright = 1.0f;
		};

		struct DynamicState
		{
			constexpr DynamicState() = default;

			IMaterial* m_BoundMaterial = nullptr;
			int m_BoneCount = 0;
			ShaderAPITextureHandle_t m_FullScreenTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
			uint_fast8_t m_AnisotropicLevel = 0;
			Util::InPlaceVector<ShaderViewport_t, 4> m_Viewports;
			float m_ClearColor[4] = {};

			StdVtxShaderConstants m_VtxShaderConstants;
		};

		const DynamicState& GetDynamicState() const { return m_State; }

	private:
		DynamicState m_State;
		bool m_Dirty = true;
	};

	extern IStateManagerDynamic& g_StateManagerDynamic;
}
