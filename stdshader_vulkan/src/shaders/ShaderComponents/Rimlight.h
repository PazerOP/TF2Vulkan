#pragma once

#include "BaseShaderComponent.h"
#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct Rimlight
	{
		struct SpecConstBuf
		{
			bool32 RIMLIGHT;
			bool32 RIMLIGHTMASK;
		};
		template<typename T> struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, RIMLIGHT);
			SPEC_CONST_BUF_ENTRY(T, RIMLIGHTMASK);
		};

		struct UniformBuf
		{
			float1 m_RimlightExponent;
			float1 m_RimlightBoost;
		};

		struct Params
		{
			NSHADER_PARAM(RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting");
			NSHADER_PARAM(RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights");
			NSHADER_PARAM(RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights");
			NSHADER_PARAM(RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term");

		private:
			template<typename... TGroups> friend class ShaderComponents;

			void InitParamGroup(IMaterialVar** params) const
			{
				if (!params[RIMLIGHT]->GetBoolValue())
					params[RIMMASK]->SetBoolValue(false);
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf) const
			{
				if (specConstBuf->RIMLIGHT = params[RIMLIGHT]->GetBoolValue())
				{
					specConstBuf->RIMLIGHTMASK = params[RIMMASK]->GetBoolValue();
					uniformBuf->m_RimlightExponent = params[RIMLIGHTEXPONENT]->GetFloatValue();
					uniformBuf->m_RimlightBoost = params[RIMLIGHTBOOST]->GetFloatValue();
				}
			}
		};
	};
} } }
