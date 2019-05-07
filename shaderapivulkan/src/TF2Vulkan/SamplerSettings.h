#pragma once

#include <TF2Vulkan/Util/std_compare.h>

#include <shaderapi/shareddefs.h>

namespace TF2Vulkan
{
	union SamplerSettings final
	{
		constexpr SamplerSettings() :
			m_MinFilter(SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR),
			m_MagFilter(SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR),

			m_WrapS(SHADER_TEXWRAPMODE_REPEAT),
			m_WrapT(SHADER_TEXWRAPMODE_REPEAT),
			m_WrapU(SHADER_TEXWRAPMODE_REPEAT)
		{
		}

#ifndef __INTELLISENSE__
		constexpr auto operator<=>(const SamplerSettings& s) const
		{
			return m_BackingValue <=> s.m_BackingValue;
		}
#endif
		constexpr bool operator==(const SamplerSettings& s) const
		{
			return m_BackingValue == s.m_BackingValue;
		}

		struct
		{
			ShaderTexFilterMode_t m_MinFilter : 3;
			ShaderTexFilterMode_t m_MagFilter : 3;

			ShaderTexWrapMode_t m_WrapS : 2;
			ShaderTexWrapMode_t m_WrapT : 2;
			ShaderTexWrapMode_t m_WrapU : 2;
		};
		int m_BackingValue;
	};

	static_assert(sizeof(SamplerSettings::m_BackingValue) == sizeof(SamplerSettings));
}

STD_HASH_DEFINITION(TF2Vulkan::SamplerSettings,
	v.m_BackingValue
);
