#pragma once

#include "ShaderDataShared.h"

#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan{ namespace ShaderConstants
{
	struct VSMatrices final
	{
		constexpr VSMatrices() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(VSMatrices);

		float4x4 m_ModelViewProj;
		float4x4 m_ViewProj;

		float4 m_ModelViewProjZ;
		float4 m_ViewProjZ;
	};

	struct VSModelMatrices final
	{
		constexpr VSModelMatrices() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(VSModelMatrices);

		std::array<float4x3, 53> m_Model;
	};

	struct VSCommon final
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

		FogParams m_FogParams;

		std::array<LightInfo, 4> m_LightInfo;
		AmbientLightCube m_AmbientCube;
	};

	struct PSCommon final
	{
		constexpr PSCommon() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(PSCommon);

		float4 m_SelfIllumTint;
		float4 m_DiffuseModulation;
		float4 m_EnvmapTintShadowTweaks;
		AmbientLightCube m_AmbientCube;
		std::array<PixelShaderLightInfo, 3> m_LightInfo;
	};

	union VSCustom final
	{
		struct XLitGenericShared
		{
			float4 m_BaseTexCoordTransform[2];
			float4 m_DetailTexCoordTransform[2];
			float3 m_MorphTargetTextureDim;
			float4 m_MorphSubrect;
		};

		struct : XLitGenericShared
		{
			float1 m_SeamlessScale;

		} m_XLitGeneric;

		struct : XLitGenericShared
		{
			float4x4 m_FlashlightWorldToTexture;
		} m_XLitGenericBump;
	};

	struct VSData final
	{
		constexpr VSData() = default;
		VSCommon m_Common;
		VSCustom m_Custom;
		VSModelMatrices m_ModelMatrices;
	};

	union PSCustom final
	{
		struct XLitGenericShared
		{
			float4 m_EnvmapTint_TintReplaceFactor;
			float4 m_DiffuseModulation;
			float4 m_EnvmapContrast_ShadowTweaks;
			float4 m_SelfIllumTint_and_BlendFactor;
			float4 m_EyePos;
			float4 m_FogParams;

			float4 m_ShaderControls;

			float4 m_FlashlightAttenuationFactors;
			float3 m_FlashlightPos;
			float4x4 m_FlashlightWorldToTexture;
		};

		struct : XLitGenericShared
		{
			float4 m_EnvmapSaturation_SelfIllumMask;

			float4 m_DepthFeatheringConstants;

		} m_XLitGeneric;

		struct : XLitGenericShared
		{
			float3 m_EnvmapSaturation;
			AmbientLightCube m_AmbientCube;
			float4 m_SelfIllumScaleBiasExpBrightness;
			PixelShaderLightInfo m_LightInfo[3];

		} m_XLitGenericBump;
	};

	struct PSData final
	{
		constexpr PSData() = default;
		PSCommon m_Common;
		PSCustom m_Custom;
	};
} }
