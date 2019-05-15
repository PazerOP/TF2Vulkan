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

	struct CommonSpecConstLayoutInfo : BaseSpecConstLayoutInfo<CommonSpecConstLayoutInfo, CommonSpecConstLayout>
	{
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, VERTEXCOLOR);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, CUBEMAP);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, HALFLAMBERT);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, FLASHLIGHT);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, SEAMLESS_BASE);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, SEAMLESS_DETAIL);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, SEPARATE_DETAIL_UVS);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, DECAL);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, DONT_GAMMA_CONVERT_VERTEX_COLOR);

		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, DYNAMIC_LIGHT);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, STATIC_LIGHT_VERTEX);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, STATIC_LIGHT_LIGHTMAP);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, DOWATERFOG);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, LIGHTING_PREVIEW);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, MORPHING);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, NORMALMAPPING);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, DIFFUSELIGHTING);
		SPEC_CONST_BUF_ENTRY(TF2VGenericSpecConsts, SELFILLUM);

	} static constexpr s_SpecConstBufLayout;
} }
