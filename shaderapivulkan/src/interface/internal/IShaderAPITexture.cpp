#include "IShaderAPITexture.h"
#include "TF2Vulkan/FormatInfo.h"

using namespace TF2Vulkan;

const vk::ImageView& IVulkanTexture::FindOrCreateView()
{
	const auto& imgCreateInfo = GetImageCreateInfo();

	vk::ImageViewCreateInfo ci;
	ci.format = imgCreateInfo.format;
	ci.image = GetImage();

	switch (imgCreateInfo.imageType)
	{
	case vk::ImageType::e1D:
	//	ci.viewType = vk::ImageViewType::e1D;
	//	break;
	case vk::ImageType::e2D:
		ci.viewType = vk::ImageViewType::e2D;
		break;
	case vk::ImageType::e3D:
		ci.viewType = vk::ImageViewType::e3D;
		break;

	default:
		throw VulkanException("Unknown vk::ImageType", EXCEPTION_DATA());
	}

	ci.subresourceRange.aspectMask = TF2Vulkan::GetAspects(ci.format);
	ci.subresourceRange.layerCount = imgCreateInfo.arrayLayers;
	ci.subresourceRange.levelCount = imgCreateInfo.mipLevels;

	return FindOrCreateView(ci);
}

int IShaderAPITexture::GetActualWidth() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(GetImageCreateInfo().extent.width);
}

int IShaderAPITexture::GetActualHeight() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(GetImageCreateInfo().extent.height);
}

int IShaderAPITexture::GetActualDepth() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(GetImageCreateInfo().extent.depth);
}

bool IShaderAPITexture::IsMipmapped() const
{
	LOG_FUNC();
	return GetImageCreateInfo().mipLevels > 1;
}

ImageFormat IShaderAPITexture::GetImageFormat() const
{
	LOG_FUNC();
	return ConvertImageFormat(GetImageCreateInfo().format);
}
