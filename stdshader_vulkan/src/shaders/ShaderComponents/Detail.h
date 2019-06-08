#pragma once

#include "BaseShaderComponent.h"
#include "shaders/UniformBufConstructs/TextureTransform.h"
#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct Detail
	{
		struct SpecConstBuf
		{
			int1 TEXINDEX_DETAIL = TEX_DEFAULT_COLOR_BLACK;
			uint1 SMPINDEX_DETAIL;
			uint1 DETAILBLENDMODE;
			bool32 SEPARATEDETAILUVS;
		};
		template<typename T> struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, TEXINDEX_DETAIL);
			SPEC_CONST_BUF_ENTRY(T, SMPINDEX_DETAIL);
			SPEC_CONST_BUF_ENTRY(T, DETAILBLENDMODE);
			SPEC_CONST_BUF_ENTRY(T, SEPARATEDETAILUVS);
		};

		struct UniformBuf
		{
			float3 m_DetailTint;
			float1 m_DetailScale;
			TextureTransform m_DetailTextureTransform;
			float1 m_DetailBlendFactor;
		};

		struct Params
		{
			NSHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture");
			NSHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail");
			NSHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture");
			NSHADER_PARAM(DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade");
			NSHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.");
			NSHADER_PARAM(DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint");
			NSHADER_PARAM(DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform");
			NSHADER_PARAM(SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture");

		private:
			template<typename... TGroups> friend class ShaderComponents;

			void InitParamGroup(IMaterialVar** params) const
			{
				if (!params[DETAIL]->IsDefined())
				{
					params[DETAILBLENDMODE]->SetIntValue(0);
					params[SEPARATEDETAILUVS]->SetBoolValue(false);
				}
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf, ShaderTextureBinder& tb) const
			{
				if (params[DETAIL]->IsTexture())
				{
					tb.AddBinding(*params[DETAIL]->GetTextureValue(), specConstBuf->TEXINDEX_DETAIL, specConstBuf->SMPINDEX_DETAIL);

					uniformBuf->m_DetailScale = params[DETAILSCALE]->GetFloatValue();
					specConstBuf->DETAILBLENDMODE = params[DETAILBLENDMODE]->GetIntValue();
					uniformBuf->m_DetailBlendFactor = params[DETAILBLENDFACTOR]->GetFloatValue();
					uniformBuf->m_DetailTint.SetFrom(params[DETAILBLENDFACTOR]->GetVecValue());
					uniformBuf->m_DetailTextureTransform = TextureTransform(params[DETAILTEXTURETRANSFORM]->GetMatrixValue());
					specConstBuf->SEPARATEDETAILUVS = params[SEPARATEDETAILUVS]->GetBoolValue();
				}
			}

			void LoadResources(IMaterialVar** params, IShaderInit& init, const char* materialName, const char* texGroupName) const
			{
				if (params[DETAIL]->IsDefined())
				{
					init.LoadTexture(params[DETAIL], texGroupName);
				}
			}
		};
	};
} } }
