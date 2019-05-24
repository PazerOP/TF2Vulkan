#pragma once

#include "ShaderDataShared.h"

#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan{ namespace Shaders
{
	struct alignas(0x100) VSModelMatrices final
	{
		constexpr VSModelMatrices() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(VSModelMatrices);

		matrix3x4_t& operator[](size_t i) { return m_Model.at(i); }
		const matrix3x4_t& operator[](size_t i) const { return m_Model.at(i); }

		std::array<matrix3x4_t, 53> m_Model;
	};

	struct alignas(0x100) ShaderDataCommon final
	{
		constexpr ShaderDataCommon() = default;
		//DEFAULT_WEAK_EQUALITY_OPERATOR(ShaderDataCommon);

		VMatrix m_ModelViewProj;
		VMatrix m_ViewProj;

		float m_OOGamma = {}; // One over gamma
		float m_OneThird = {};
		int m_LightCount = {};
		bool4 m_LightEnabled;

		Vector m_EyePos;
		float m_WaterZ = {};

		float4 m_FlexScale;

		FogParams m_FogParams;

		std::array<LightInfo, 4> m_LightInfo;
		AmbientLightCube m_AmbientCube;

		float4 m_SelfIllumTint;
		float4 m_DiffuseModulation;
		float4 m_EnvmapTintShadowTweaks;
	};
} }
