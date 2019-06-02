#pragma once

#include "BaseShaderComponent.h"
#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders{ namespace Components
{
	struct Phong
	{
		struct SpecConstBuf
		{
			bool32 PHONG;
			bool32 PHONG_ALBEDO_TINT;
			bool32 PHONG_INVERT_MASK;
			bool32 PHONG_MAP_FROM_BASEALPHA;
		};
		template<typename T> struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, PHONG);
			SPEC_CONST_BUF_ENTRY(T, PHONG_INVERT_MASK);
			SPEC_CONST_BUF_ENTRY(T, PHONG_MAP_FROM_BASEALPHA);
		};

		struct UniformBuf
		{
			float1 m_PhongExponent;
			float3 m_PhongTint;
			float3 m_PhongAlbedoTint;
			float1 m_PhongExponentFactor;
			float1 m_PhongBoost;
		};

		struct Params
		{
			NSHADER_PARAM(PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting");
			NSHADER_PARAM(PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights");
			NSHADER_PARAM(PHONGTINT, SHADER_PARAM_TYPE_VEC3, "[1 1 1]", "Phong tint for local specular lights");
			NSHADER_PARAM(PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1", "Apply tint by albedo (controlled by spec exponent texture");
			NSHADER_PARAM(INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)");
			NSHADER_PARAM(BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha");
			NSHADER_PARAM(PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term");
			NSHADER_PARAM(PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0 0.5 1]", "Parameters for remapping fresnel output");
			NSHADER_PARAM(PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)");
			NSHADER_PARAM(PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map");
			NSHADER_PARAM(PHONGEXPONENTFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "When using a phong exponent texture, this will be multiplied by the 0..1 that comes out of the texture.");

		private:
			template<typename... TGroups> friend class ShaderComponents;

			void InitParamGroup(IMaterialVar** params) const
			{
			}

			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf) const
			{
				if (specConstBuf->PHONG = params[PHONG]->GetBoolValue())
				{
					uniformBuf->m_PhongExponent = params[PHONGEXPONENT]->GetFloatValue();
					uniformBuf->m_PhongExponentFactor = params[PHONGEXPONENTFACTOR]->GetFloatValue();
					uniformBuf->m_PhongTint.SetFrom(params[PHONGTINT]->GetVecValue());

					specConstBuf->PHONG_ALBEDO_TINT = params[PHONGALBEDOTINT]->GetBoolValue();
					specConstBuf->PHONG_INVERT_MASK = params[INVERTPHONGMASK]->GetBoolValue();
					specConstBuf->PHONG_MAP_FROM_BASEALPHA = params[BASEMAPALPHAPHONGMASK]->GetBoolValue();
				}
			}
		};
	};
} } }
