#pragma once

#include "BaseShaderComponent.h"
#include <shaderlib/cshader.h>

#include "shaders/UniformBufConstructs/TextureTransform.h"

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct Bumpmap
	{
		struct SpecConstBuf
		{
			int1 TEXINDEX_BUMPMAP = TEX_DEFAULT_COLOR_FLATNORMAL;
			uint1 SMPINDEX_BUMPMAP;
			int1 TEXINDEX_BUMPCOMPRESS = TEX_DEFAULT_COLOR_BLACK;
			uint1 SMPINDEX_BUMPCOMPRESS;
			int1 TEXINDEX_BUMPSTRETCH = TEX_DEFAULT_COLOR_BLACK;
			uint1 SMPINDEX_BUMPSTRETCH;
		};
		template<typename T> struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, TEXINDEX_BUMPMAP);
			SPEC_CONST_BUF_ENTRY(T, SMPINDEX_BUMPMAP);
			SPEC_CONST_BUF_ENTRY(T, TEXINDEX_BUMPCOMPRESS);
			SPEC_CONST_BUF_ENTRY(T, SMPINDEX_BUMPCOMPRESS);
			SPEC_CONST_BUF_ENTRY(T, TEXINDEX_BUMPSTRETCH);
			SPEC_CONST_BUF_ENTRY(T, SMPINDEX_BUMPSTRETCH);
		};

		struct UniformBuf
		{
			TextureTransform m_BumpTransform;
		};

		struct Params
		{
			NSHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map");
			NSHADER_PARAM(BUMPCOMPRESS, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "compression bump map");
			NSHADER_PARAM(BUMPSTRETCH, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "expansion bump map");
			NSHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap");
			NSHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform");

		private:
			template<typename... TGroups> friend class ShaderComponents;

			void InitParamGroup(IMaterialVar** params) const
			{
				if (!g_pConfig->UseBumpmapping())
				{
					params[BUMPMAP]->SetUndefined();
					return;
				}

				if (params[BUMPMAP]->IsDefined())
					SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf, ShaderTextureBinder& tb) const
			{
				if (params[BUMPMAP]->IsTexture())
				{
					tb.AddBinding(*params[BUMPMAP]->GetTextureValue(), specConstBuf->TEXINDEX_BUMPMAP, specConstBuf->SMPINDEX_BUMPMAP);
					uniformBuf->m_BumpTransform = TextureTransform(params[BUMPTRANSFORM]->GetMatrixValue());

					if (params[BUMPCOMPRESS]->IsTexture() && params[BUMPSTRETCH]->IsTexture())
					{
						tb.AddBinding(*params[BUMPCOMPRESS]->GetTextureValue(), specConstBuf->TEXINDEX_BUMPCOMPRESS, specConstBuf->SMPINDEX_BUMPCOMPRESS);
						tb.AddBinding(*params[BUMPSTRETCH]->GetTextureValue(), specConstBuf->TEXINDEX_BUMPSTRETCH, specConstBuf->SMPINDEX_BUMPCOMPRESS);
					}
				}
			}

			void LoadResources(IMaterialVar** params, IShaderInit& init, const char* materialName, const char* texGroupName) const
			{
				if (g_pConfig->UseBumpmapping())
				{
					init.LoadBumpMap(params[BUMPMAP], texGroupName);

					if (params[BUMPCOMPRESS]->IsDefined() && params[BUMPSTRETCH]->IsDefined())
					{
						init.LoadTexture(params[BUMPCOMPRESS], texGroupName);
						init.LoadTexture(params[BUMPSTRETCH], texGroupName);
					}
				}
			}
		};
	};
} } }
