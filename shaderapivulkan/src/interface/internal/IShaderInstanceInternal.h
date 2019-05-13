#pragma once

#include <TF2Vulkan/IShaderInstance.h>

namespace TF2Vulkan
{
	class IVulkanShader;

	class IShaderInstanceInternal : public virtual IShaderInstance
	{
	public:
		virtual const IVulkanShader& GetVulkanShader() const = 0;
		virtual void CreateSpecializationInfo(vk::SpecializationInfo& info,
			std::vector<vk::SpecializationMapEntry>& entries, std::vector<std::byte>& data) const = 0;
	};
	class IPSInstanceInternal : public IPSInstance, public virtual IShaderInstanceInternal
	{

	};
	class IVSInstanceInternal : public IVSInstance, public virtual IShaderInstanceInternal
	{

	};
}
