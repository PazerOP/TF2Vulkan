#pragma once

#include "BaseShaderComponent.h"
#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct WeaponSheen
	{
		struct SpecConstBuf
		{
			uint1 SHEEN_EFFECT_TYPE;
		};
		template<typename T> struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, SHEEN_EFFECT_TYPE);
		};

		struct UniformBuf
		{
			float2 m_SheenScale;
			float2 m_SheenOffset;
			float3 m_SheenTint;
		};

		struct Params
		{
			NSHADER_PARAM(SHEENPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables weapon sheen render in a second pass");
			NSHADER_PARAM(SHEENMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "sheenmap");
			NSHADER_PARAM(SHEENMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "sheenmap mask");
			NSHADER_PARAM(SHEENMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "");
			NSHADER_PARAM(SHEENMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "sheenmap tint");
			NSHADER_PARAM(SHEENMAPMASKSCALEX, SHADER_PARAM_TYPE_FLOAT, "1", "X Scale the size of the map mask to the size of the target");
			NSHADER_PARAM(SHEENMAPMASKSCALEY, SHADER_PARAM_TYPE_FLOAT, "1", "Y Scale the size of the map mask to the size of the target");
			NSHADER_PARAM(SHEENMAPMASKOFFSETX, SHADER_PARAM_TYPE_FLOAT, "0", "X Offset of the mask relative to model space coords of target");
			NSHADER_PARAM(SHEENMAPMASKOFFSETY, SHADER_PARAM_TYPE_FLOAT, "0", "Y Offset of the mask relative to model space coords of target");
			NSHADER_PARAM(SHEENMAPMASKDIRECTION, SHADER_PARAM_TYPE_INTEGER, "0", "The direction the sheen should move (length direction of weapon) XYZ, 0,1,2");
			NSHADER_PARAM(SHEENINDEX, SHADER_PARAM_TYPE_INTEGER, "0", "Index of the Effect Type (Color Additive, Override etc...)");

		private:
			template<typename... TGroups> friend class ShaderComponents;

			void InitParamGroup(IMaterialVar** params) const
			{
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf) const
			{
				if ((specConstBuf->SHEEN_EFFECT_TYPE = params[SHEENPASSENABLED]->GetBoolValue() ? (params[SHEENINDEX]->GetIntValue() + 1) : 0) != 0U)
				{
					uniformBuf->m_SheenScale.x = params[SHEENMAPMASKSCALEX]->GetFloatValue();
					uniformBuf->m_SheenScale.y = params[SHEENMAPMASKSCALEY]->GetFloatValue();
					uniformBuf->m_SheenOffset.x = params[SHEENMAPMASKOFFSETX]->GetFloatValue();
					uniformBuf->m_SheenOffset.y = params[SHEENMAPMASKOFFSETY]->GetFloatValue();
					uniformBuf->m_SheenTint.SetFrom(params[SHEENMAPTINT]->GetVecValue());
				}
			}
		};
	};
} } }
