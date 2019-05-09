#pragma once

#include "ShaderDataShared.h"

#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan{ namespace ShaderConstants
{
	struct VSCommon
	{
		constexpr VSCommon() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(VSCommon);

		float m_OOGamma = {}; // One over gamma
		float m_OneThird = {};
		int m_LightCount = {};
		bool4 m_LightEnabled;

		float3 m_EyePos;
		float m_WaterZ = {};

		float4 m_FlexScale;

		float4x4 m_ModelViewProj;
		float4x4 m_ViewProj;

		float4 m_ModelViewProjZ;
		float4 m_ViewProjZ;

		FogParams m_FogParams;

		LightInfo m_LightInfo[4];
		AmbientLightCube m_AmbientCube;

		float4x3 m_Model[53];
	};
} }
