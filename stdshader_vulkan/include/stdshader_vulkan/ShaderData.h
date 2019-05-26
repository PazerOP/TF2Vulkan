#pragma once

#include "ShaderDataShared.h"

#include <TF2Vulkan/Util/Macros.h>
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
		uint32_t m_LightCount = {};
		bool4 m_LightEnabled;

		Vector m_EyePos;
		float m_WaterZ = {};

		float4 m_FlexScale;

		FogParams m_FogParams;

		std::array<LightInfo, 4> m_LightInfo;
		AmbientLightCube m_AmbientCube;

		float4 m_LinearFogColor;
		float4 m_LightScale;
	};

	CHECK_OFFSET(ShaderDataCommon, m_ModelViewProj, 0);          // 0
	CHECK_OFFSET(ShaderDataCommon, m_ViewProj, 64);              // 1
	CHECK_OFFSET(ShaderDataCommon, m_OOGamma, 128);              // 2
	CHECK_OFFSET(ShaderDataCommon, m_LightCount, 132);           // 3
	CHECK_OFFSET(ShaderDataCommon, m_LightEnabled, 144);         // 4
	CHECK_OFFSET(ShaderDataCommon, m_EyePos, 160);               // 5
	CHECK_OFFSET(ShaderDataCommon, m_WaterZ, 172);               // 6
	CHECK_OFFSET(ShaderDataCommon, m_FlexScale, 176);            // 7
	CHECK_OFFSET(ShaderDataCommon, m_FogParams, 192);            // 8
	CHECK_OFFSET(ShaderDataCommon, m_LightInfo, 208);            // 9
	CHECK_OFFSET(ShaderDataCommon, m_AmbientCube, 528);          // 10
	CHECK_OFFSET(ShaderDataCommon, m_LinearFogColor, 624);       // 11
	CHECK_OFFSET(ShaderDataCommon, m_LightScale, 640);           // 12
} }
