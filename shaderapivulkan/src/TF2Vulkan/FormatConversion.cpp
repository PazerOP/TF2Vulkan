#include "FormatConversion.h"
#include "Globals.h"
#include "IShaderDeviceMgrInternal.h"

#include <bitmap/imageformat.h>

using namespace TF2Vulkan;

vk::Format TF2Vulkan::ConvertImageFormat(ImageFormat format)
{
	switch (format)
	{
	default: assert(!"Unknown format");
	case IMAGE_FORMAT_UNKNOWN:  return vk::Format::eUndefined;

	case IMAGE_FORMAT_RGBA8888: return vk::Format::eR8G8B8A8Unorm;
	case IMAGE_FORMAT_BGRA8888: return vk::Format::eB8G8R8A8Unorm;
	case IMAGE_FORMAT_ABGR8888: return vk::Format::eA8B8G8R8UnormPack32;

	case IMAGE_FORMAT_RGB888:   return vk::Format::eR8G8B8Unorm;
	case IMAGE_FORMAT_BGR888:   return vk::Format::eB8G8R8Unorm;
	case IMAGE_FORMAT_RGB565:   return vk::Format::eR5G6B5UnormPack16;

	case IMAGE_FORMAT_I8:       return vk::Format::eR8Unorm;
	}
}

ImageFormat TF2Vulkan::ConvertImageFormat(vk::Format format)
{
	switch (format)
	{
	default: assert(!"Unknown format");
	case vk::Format::eUndefined:            return IMAGE_FORMAT_UNKNOWN;

	case vk::Format::eR8G8B8A8Unorm:        return IMAGE_FORMAT_RGBA8888;
	case vk::Format::eA8B8G8R8UnormPack32:  return IMAGE_FORMAT_ABGR8888;
	}
}

static ImageFormat FindSupportedFormat(FormatUsage usage, bool filtering, std::initializer_list<ImageFormat> formats)
{
	for (auto fmt : formats)
	{
		if (TF2Vulkan::HasHardwareSupport(fmt, usage, filtering))
			return fmt;
	}

	assert(!"No supported alternatives found!");
	return IMAGE_FORMAT_UNKNOWN;
}

ImageFormat TF2Vulkan::PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering)
{
#define PROMOTE_2_HARDWARE(baseFmt, ...) case baseFmt : return FindSupportedFormat(usage, filtering, { baseFmt, __VA_ARGS__ })

	switch (format)
	{
		PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGB888,
			IMAGE_FORMAT_BGR888,
			IMAGE_FORMAT_RGBA8888,
			IMAGE_FORMAT_ARGB8888);

		PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGRA8888,
			IMAGE_FORMAT_RGBA8888,
			IMAGE_FORMAT_ARGB8888);
	}

	assert(!"Unknown format");
	return IMAGE_FORMAT_UNKNOWN;
}

bool TF2Vulkan::HasHardwareSupport(ImageFormat format, FormatUsage usage, bool filtering)
{
	auto vkFormat = ConvertImageFormat(format);
	if (vkFormat == vk::Format::eUndefined)
		return false;

	auto formatProperties = g_ShaderDeviceMgr.GetAdapter().getFormatProperties(vkFormat);

	using Flags = vk::FormatFeatureFlagBits;

	vk::FormatFeatureFlags REQUIRED_FLAGS{};

	if (usage == FormatUsage::ImmutableTexture || usage == FormatUsage::RenderTarget)
	{
		if (filtering)
			REQUIRED_FLAGS |= Flags::eSampledImageFilterLinear;
		else
			REQUIRED_FLAGS |= Flags::eSampledImage;

		if (usage == FormatUsage::RenderTarget)
		{
			REQUIRED_FLAGS |= Flags::eStorageImage;
		}
	}
	else
	{
		assert(!"Unknown usage");
		return false;
	}

	const auto andResult = (formatProperties.optimalTilingFeatures & REQUIRED_FLAGS);

	const bool hasSupport = andResult == REQUIRED_FLAGS;
	assert(hasSupport);
	return hasSupport;
}
