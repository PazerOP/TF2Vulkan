#pragma once

#include "BaseShaderComponent.h"
#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct MYCOMPONENT
	{
		struct SpecConstBuf
		{
		};
		template<typename T> struct SpecConstLayout
		{
		};

		struct UniformBuf
		{
		};

		struct Params
		{

		private:
			template<typename... TGroups> friend class ShaderParams;

			void InitParamGroup(IMaterialVar** params) const
			{
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf) const
			{
			}
		};
	};
} } }
