#pragma once

#include <TF2Vulkan/IShaderGroup.h>
#include <TF2Vulkan/IShaderInstance.h>

namespace TF2Vulkan
{
	class IVulkanShader;

	class IShaderGroupInternal : public IShaderGroup
	{
	public:
		virtual const IVulkanShader& GetVulkanShader() const = 0;
	};

	class IShaderInstanceInternal : public IShaderInstance
	{
	public:
		virtual const IShaderGroupInternal& GetGroup() const override = 0;
		virtual void GetSpecializationInfo(vk::SpecializationInfo& info) const = 0;

		void GetResourceCounts(uint32_t& texture2DCount, uint32_t& samplerCount) const
		{
			texture2DCount = 0;
			samplerCount = 0;

			const std::byte* specConstBuf = reinterpret_cast<const std::byte*>(GetSpecConstBuffer());
			for (const auto& entry : GetGroup().GetSpecConstLayout().GetCreateInfo())
			{
				if (!texture2DCount && entry.m_Name == "TEXTURE2D_COUNT"sv)
				{
					texture2DCount = *reinterpret_cast<const uint32_t*>(&specConstBuf[entry.m_Offset]);
					if (samplerCount)
						break;
				}
				else if (!samplerCount && entry.m_Name == "SAMPLER_COUNT"sv)
				{
					samplerCount = *reinterpret_cast<const uint32_t*>(&specConstBuf[entry.m_Offset]);
					if (texture2DCount)
						break;
				}
			}
		}
	};
}
