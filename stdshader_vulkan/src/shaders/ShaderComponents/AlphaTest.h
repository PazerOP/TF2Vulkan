#pragma once

#include "BaseShaderComponent.h"
#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct AlphaTest
	{
		struct SpecConstBuf
		{
			bool32 VERTEXALPHATEST;
		};
		template<typename T> struct SpecConstLayout
		{
		};

		struct UniformBuf
		{
			float1 m_AlphaTestReference;
		};

		struct Params
		{
			NSHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "");
			NSHADER_PARAM(VERTEXALPHATEST, SHADER_PARAM_TYPE_INTEGER, "0", "");

		private:
			template<typename... TGroups> friend class ShaderComponents;

			void InitParamGroup(IMaterialVar** params) const
			{
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf) const
			{
				uniformBuf->m_AlphaTestReference = params[ALPHATESTREFERENCE]->GetFloatValue();
			}
		};
	};
} } }
