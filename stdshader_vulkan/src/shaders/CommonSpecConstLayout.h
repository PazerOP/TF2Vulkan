#pragma once

#include <TF2Vulkan/ISpecConstLayout.h>

namespace TF2Vulkan{ namespace Shaders
{
	struct CommonSpecConstLayout : BaseSpecConstBuffer<CommonSpecConstLayout>
	{
		bool32 VERTEXCOLOR = false;
		bool32 CUBEMAP = false;
		bool32 HALFLAMBERT = false;
		bool32 FLASHLIGHT = false;
		bool32 SEAMLESS_BASE = false;
		bool32 SEAMLESS_DETAIL = false;
		bool32 SEPARATE_DETAIL_UVS = false;
		bool32 DECAL = false;
		bool32 DONT_GAMMA_CONVERT_VERTEX_COLOR = false;

		bool32 DYNAMIC_LIGHT = false;
		bool32 STATIC_LIGHT_VERTEX = false;
		bool32 STATIC_LIGHT_LIGHTMAP = false;
		int1 DOWATERFOG = 0;
		bool32 LIGHTING_PREVIEW = false;
		bool32 MORPHING = false;
		bool32 NORMALMAPPING = false;
		bool32 DIFFUSELIGHTING = false;
		bool32 SELFILLUM = false;
	};

	struct CommonSpecConstLayoutInfo : BaseSpecConstLayout<CommonSpecConstLayoutInfo, CommonSpecConstLayout>
	{
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, VERTEXCOLOR);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, CUBEMAP);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, HALFLAMBERT);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, FLASHLIGHT);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, SEAMLESS_BASE);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, SEAMLESS_DETAIL);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, SEPARATE_DETAIL_UVS);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, DECAL);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, DONT_GAMMA_CONVERT_VERTEX_COLOR);

		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, DYNAMIC_LIGHT);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, STATIC_LIGHT_VERTEX);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, STATIC_LIGHT_LIGHTMAP);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, DOWATERFOG);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, LIGHTING_PREVIEW);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, MORPHING);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, NORMALMAPPING);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, DIFFUSELIGHTING);
		SPEC_CONST_BUF_ENTRY(CommonSpecConstLayout, SELFILLUM);

	} static constexpr s_SpecConstBufLayout;
} }
