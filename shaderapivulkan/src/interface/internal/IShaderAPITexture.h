#pragma once

#include <shaderapi/ishaderdynamic.h>

namespace TF2Vulkan
{
	class IVulkanTexture
	{
	protected:
		virtual ~IVulkanTexture() = default;

	public:
		virtual std::string_view GetDebugName() const = 0;
		virtual const vk::Image& GetImage() const = 0;
		virtual const vk::ImageCreateInfo& GetImageCreateInfo() const = 0;

		virtual const vk::ImageView& FindOrCreateView(const vk::ImageViewCreateInfo& createInfo) = 0;

		virtual const vk::ImageView& FindOrCreateView();

		void GetSize(uint32_t& width, uint32_t& height) const
		{
			const auto& ci = GetImageCreateInfo();
			width = ci.extent.width;
			height = ci.extent.height;
		}
	};

	class IShaderAPITexture : public IVulkanTexture
	{
	public:
		virtual ShaderAPITextureHandle_t GetHandle() const = 0;
	};
}
