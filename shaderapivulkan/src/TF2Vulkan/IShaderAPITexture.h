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

		virtual const vk::ImageView& FindOrCreateView()
		{
			const auto& imgCreateInfo = GetImageCreateInfo();

			vk::ImageViewCreateInfo ci;
			ci.format = imgCreateInfo.format;
			ci.image = GetImage();

			switch (imgCreateInfo.imageType)
			{
			case vk::ImageType::e1D:
				ci.viewType = vk::ImageViewType::e1D;
				break;
			case vk::ImageType::e2D:
				ci.viewType = vk::ImageViewType::e2D;
				break;
			case vk::ImageType::e3D:
				ci.viewType = vk::ImageViewType::e3D;
				break;

			default:
				throw VulkanException("Unknown vk::ImageType", EXCEPTION_DATA());
			}

			ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			ci.subresourceRange.layerCount = imgCreateInfo.arrayLayers;
			ci.subresourceRange.levelCount = imgCreateInfo.mipLevels;

			return FindOrCreateView(ci);
		}

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
