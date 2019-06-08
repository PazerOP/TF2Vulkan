#pragma once

#include "AlignedTypes.h"
#include "TF2Vulkan/SamplerSettings.h"
#include "TF2Vulkan/Util/InPlaceVector.h"
#include "TF2Vulkan/Util/SafeConvert.h"

#include <materialsystem/itexture.h>
#include <vtf/vtf.h>

namespace TF2Vulkan
{
	static constexpr size_t MAX_SHADER_RESOURCE_BINDINGS = 32;

	class ShaderTextureBinder final
	{
	public:
		void AddBinding(ITexture& texture, int32_t& texLocationSpecConst, uint32_t& smpLocationSpecConst,
			const SamplerSettings* sampler = nullptr);

		const auto& GetTextures() const { return m_Textures; }
		const auto& GetSamplers() const { return m_Samplers; }

	private:
		int32_t FindOrInsertTexture(ITexture& texture);
		uint32_t FindOrInsertSampler(const SamplerSettings& sampler);
		static SamplerSettings SamplerFromTexture(const ITexture& texture);

		Util::InPlaceVector<ITexture*, MAX_SHADER_RESOURCE_BINDINGS> m_Textures;
		Util::InPlaceVector<SamplerSettings, MAX_SHADER_RESOURCE_BINDINGS> m_Samplers;
	};

	inline void ShaderTextureBinder::AddBinding(ITexture& texture, int32_t& texLocationSpecConst,
		uint32_t& smpLocationSpecConst, const SamplerSettings* sampler)
	{
		texLocationSpecConst = FindOrInsertTexture(texture);
		smpLocationSpecConst = FindOrInsertSampler(sampler ? *sampler : SamplerFromTexture(texture));
	}

	inline int32_t ShaderTextureBinder::FindOrInsertTexture(ITexture& texture)
	{
		for (size_t i = 0; i < m_Textures.size(); i++)
		{
			if (m_Textures[i] == &texture)
				return Util::SafeConvert<int32_t>(i);
		}

		m_Textures.emplace_back(&texture);
		return Util::SafeConvert<int32_t>(m_Textures.size() - 1);
	}

	inline uint32_t ShaderTextureBinder::FindOrInsertSampler(const SamplerSettings& sampler)
	{
		for (uint32_t i = 0; i < m_Samplers.size(); i++)
		{
			if (m_Samplers[i] == sampler)
				return i;
		}

		m_Samplers.emplace_back(sampler);
		return m_Samplers.size() - 1;
	}

	inline SamplerSettings ShaderTextureBinder::SamplerFromTexture(const ITexture& tex)
	{
		const auto flags = tex.GetFlags();

		SamplerSettings retVal;

		// This code relies on the constructor for SamplerSettings initializing
		// these to SHADER_TEXWRAPMODE_REPEAT
		assert(retVal.m_WrapS == SHADER_TEXWRAPMODE_REPEAT);
		assert(retVal.m_WrapT == SHADER_TEXWRAPMODE_REPEAT);
		assert(retVal.m_WrapU == SHADER_TEXWRAPMODE_REPEAT);

		if (flags & TEXTUREFLAGS_CLAMPS)
			retVal.m_WrapS = SHADER_TEXWRAPMODE_CLAMP;
		if ((flags & TEXTUREFLAGS_CLAMPT) && tex.GetActualHeight() > 1)
			retVal.m_WrapT = SHADER_TEXWRAPMODE_CLAMP;
		if ((flags & TEXTUREFLAGS_CLAMPU) && tex.GetActualDepth() > 1)
			retVal.m_WrapU = SHADER_TEXWRAPMODE_CLAMP;

		if (flags & TEXTUREFLAGS_POINTSAMPLE)
		{
			retVal.m_MinFilter = retVal.m_MagFilter =
				SHADER_TEXFILTERMODE_NEAREST;
		}
		else if (flags & TEXTUREFLAGS_TRILINEAR)
		{
			retVal.m_MinFilter = retVal.m_MagFilter =
				SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR;
		}
		else if (flags & TEXTUREFLAGS_ANISOTROPIC)
		{
			retVal.m_MinFilter = retVal.m_MagFilter =
				SHADER_TEXFILTERMODE_ANISOTROPIC;
		}

		return retVal;
	}
}
