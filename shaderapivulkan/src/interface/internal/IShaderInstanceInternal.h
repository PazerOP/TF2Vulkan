#pragma once

#include <TF2Vulkan/IShaderInstance.h>

namespace TF2Vulkan
{
	class IVulkanShader;

	class IShaderGroupInternal : public IShaderGroup
	{
	public:
		virtual const IVulkanShader& GetVulkanShader() const = 0;
		//virtual const vk::SpecializationMapEntry& GetSpecMapEntries() const = 0;
	};

	class IShaderInstanceInternal : public IShaderInstance
	{
	public:
		virtual const IShaderGroupInternal& GetGroup() const override = 0;
		virtual void GetSpecializationInfo(vk::SpecializationInfo& info) const = 0;
	};
}
