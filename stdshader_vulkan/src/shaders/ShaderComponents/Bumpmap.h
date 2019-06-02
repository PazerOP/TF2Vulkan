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
		};
		template<typename T> struct SpecConstLayout
		{
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

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf) const
			{
				uniformBuf->m_BumpTransform = TextureTransform(params[BUMPTRANSFORM]->GetMatrixValue());
			}
		};
	};
} } }
