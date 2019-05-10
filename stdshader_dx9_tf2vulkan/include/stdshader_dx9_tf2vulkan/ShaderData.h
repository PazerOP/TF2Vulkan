#pragma once

#include "ShaderDataShared.h"

#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan{ namespace ShaderConstants
{
	struct alignas(0x100) VSMatrices final
	{
		constexpr VSMatrices() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(VSMatrices);

		float4x4 m_ModelViewProj;
		float4x4 m_ViewProj;

		float4 m_ModelViewProjZ;
		float4 m_ViewProjZ;
	};

	struct alignas(0x100) VSModelMatrices final
	{
		constexpr VSModelMatrices() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(VSModelMatrices);

		std::array<matrix3x4_t, 53> m_Model;
	};

	struct alignas(0x100) VSCommon final
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

	struct alignas(0x100) PSCommon final
	{
		constexpr PSCommon() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(PSCommon);

		float4 m_SelfIllumTint;
		float4 m_DiffuseModulation;
		float4 m_EnvmapTintShadowTweaks;
		AmbientLightCube m_AmbientCube;
		std::array<PixelShaderLightInfo, 3> m_LightInfo;
	};

	union alignas(0x100) VSCustom final
	{
		struct
		{
			float4 m_BaseTexCoordTransform[2];
			float4 m_DetailTexCoordTransform[2];
			float4 m_MorphSubrect;
			float3 m_MorphTargetTextureDim;
			float1 m_SeamlessScale;
			float4x4 m_FlashlightWorldToTexture;

		} m_XLitGeneric;
	};

	struct alignas(0x100) VSData final
	{
		constexpr VSData() = default;
		VSCommon m_Common;
		VSMatrices m_Matrices;
		VSCustom m_Custom;
		VSModelMatrices m_ModelMatrices;
	};

	union alignas(0x100) PSCustom final
	{
		struct
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

			float4 m_EnvmapSaturation_SelfIllumMask;

			float4 m_DepthFeatheringConstants;

			AmbientLightCube m_AmbientCube;
			float4 m_SelfIllumScaleBiasExpBrightness;
			PixelShaderLightInfo m_LightInfo[3];

		} m_XLitGeneric;
	};

	struct alignas(0x100) PSData final
	{
		constexpr PSData() = default;
		PSCommon m_Common;
		PSCustom m_Custom;
	};

	struct alignas(0x100) ShaderData final
	{
		VSData m_VSData;
		PSData m_PSData;
	};
} }
