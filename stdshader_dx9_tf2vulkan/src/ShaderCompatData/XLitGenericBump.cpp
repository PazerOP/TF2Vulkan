#include "stdshader_dx9_tf2vulkan/ShaderCompatData.h"
#include <TF2Vulkan/Util/Macros.h>

#include <../materialsystem/stdshaders/cpp_shader_constant_register_map.h>
#include <materialsystem/ishadersystem_declarations.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::ShaderCompatData;
using namespace TF2Vulkan::ShaderConstants;

namespace
{
	class XLitGenericBump final : public IShaderCompatData
	{
	public:
		void SetConstant(ShaderConstants::VSData& data, uint32_t var, const ShaderConstants::float4& vec4) const override
		{
			auto& xlit = data.m_Custom.m_XLitGenericBump;
			switch (var)
			{
			default:
				NOT_IMPLEMENTED_FUNC();

			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_0:
				xlit.m_BaseTexCoordTransform[0] = vec4;
				break;
			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_1:
				xlit.m_BaseTexCoordTransform[1] = vec4;
				break;

			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_4:
				xlit.m_DetailTexCoordTransform[0] = vec4;
				break;
			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_5:
				xlit.m_DetailTexCoordTransform[1] = vec4;
				break;

			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_6:
				xlit.m_FlashlightWorldToTexture[0] = vec4;
				break;
			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_7:
				xlit.m_FlashlightWorldToTexture[1] = vec4;
				break;
			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_8:
				xlit.m_FlashlightWorldToTexture[2] = vec4;
				break;
			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_9:
				xlit.m_FlashlightWorldToTexture[3] = vec4;
				break;

			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_10:
				xlit.m_MorphTargetTextureDim.x = vec4.x;
				xlit.m_MorphTargetTextureDim.y = vec4.y;
				xlit.m_MorphTargetTextureDim.z = vec4.z;
				break;
			case VERTEX_SHADER_SHADER_SPECIFIC_CONST_11:
				xlit.m_MorphSubrect = vec4;
				break;
			}
		}
		void SetConstant(ShaderConstants::VSData& data, uint32_t var, const ShaderConstants::int4& vec4) const override
		{
			NOT_IMPLEMENTED_FUNC();
		}
		void SetConstant(ShaderConstants::VSData& data, uint32_t var, const ShaderConstants::bool4& vec4) const override
		{
			NOT_IMPLEMENTED_FUNC();
		}
		void SetConstant(ShaderConstants::PSData& data, uint32_t var, const ShaderConstants::float4& vec4) const override
		{
			auto& xlit = data.m_Custom.m_XLitGenericBump;
			switch (var)
			{
			default:
				NOT_IMPLEMENTED_FUNC();

			case 0:
				xlit.m_EnvmapTint_TintReplaceFactor = vec4;
				break;
			case 1:
				xlit.m_DiffuseModulation = vec4;
				break;
			case 2:
				xlit.m_EnvmapContrast_ShadowTweaks = vec4;
				break;
			case 3:
				xlit.m_EnvmapSaturation.x = vec4.x;
				xlit.m_EnvmapSaturation.y = vec4.y;
				xlit.m_EnvmapSaturation.z = vec4.z;
				break;
			case 4:
				xlit.m_SelfIllumTint_and_BlendFactor = vec4;
				break;

			case 11:
				xlit.m_SelfIllumScaleBiasExpBrightness = vec4;
				break;
			case 12:
				xlit.m_ShaderControls = vec4;
				break;

			case 13:
				xlit.m_LightInfo[0].color = vec4;
				break;
			case 14:
				xlit.m_LightInfo[0].pos = vec4;
				break;
			case 15:
				xlit.m_LightInfo[1].color = vec4;
				break;
			case 16:
				xlit.m_LightInfo[1].pos = vec4;
				break;
			case 17:
				xlit.m_LightInfo[2].color = vec4;
				break;
			case 18:
				xlit.m_LightInfo[2].pos = vec4;
				break;

			case 20:
				xlit.m_EyePos = vec4;
				break;
			case 21:
				xlit.m_FogParams = vec4;
				break;
			}
		}
		void SetConstant(ShaderConstants::PSData& data, uint32_t var, const ShaderConstants::int4& vec4) const override
		{
			NOT_IMPLEMENTED_FUNC();
		}
		void SetConstant(ShaderConstants::PSData& data, uint32_t var, const ShaderConstants::bool4& vec4) const override
		{
			NOT_IMPLEMENTED_FUNC();
		}

		const SpecConstMapping* GetSpecConstMappings(size_t& count) const override
		{
			// TODO: This is just copy pasted from XLitGeneric
			NOT_IMPLEMENTED_FUNC_NOBREAK();
			static constexpr SpecConstMapping s_SpecConstMapping[] =
			{
				{ "VERTEXCOLOR", 8, },
				{ "CUBEMAP", 9 },
				{ "HALFLAMBERT", 10 },
				{ "FLASHLIGHT", 11 },
				{ "SEAMLESS_BASE", 12 },
				{ "SEAMLESS_DETAIL", 13 },
				{ "SEPARATE_DETAIL_UVS", 14 },
				{ "DONT_GAMMA_CONVERT_VERTEX_COLOR", 16 },
			};

			count = std::size(s_SpecConstMapping);
			return s_SpecConstMapping;
		}
	};
}

static const XLitGenericBump s_XLitGenericBump;
const IShaderCompatData& TF2Vulkan::ShaderCompatData::g_XLitGenericBump = s_XLitGenericBump;
