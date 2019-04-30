#include "FormatConversion.h"
#include "ShaderDeviceMgr.h"

#include <bitmap/imageformat.h>

#include <mutex>

using namespace TF2Vulkan;

vk::Format TF2Vulkan::ConvertImageFormat(ImageFormat format)
{
	switch (format)
	{
	default: assert(!"Unknown format");
	case IMAGE_FORMAT_UNKNOWN:
	case IMAGE_FORMAT_UVLX8888:
	case IMAGE_FORMAT_P8:
	case IMAGE_FORMAT_NV_DST16:
	case IMAGE_FORMAT_NV_DST24:
	case IMAGE_FORMAT_NV_INTZ:
	case IMAGE_FORMAT_NV_RAWZ:
	case IMAGE_FORMAT_ATI_DST16:
	case IMAGE_FORMAT_ATI_DST24:
	case IMAGE_FORMAT_NV_NULL:
	case IMAGE_FORMAT_ATI2N:
	case IMAGE_FORMAT_ATI1N:
		return vk::Format::eUndefined;

		// R
	case IMAGE_FORMAT_R32F:              return vk::Format::eR32Sfloat;
	case IMAGE_FORMAT_A8:
	case IMAGE_FORMAT_I8:                return vk::Format::eR8Unorm;

		// RG
	case IMAGE_FORMAT_IA88:              return vk::Format::eR8G8Unorm;
	case IMAGE_FORMAT_UV88:              return vk::Format::eR8G8Snorm;

		// RGB
	case IMAGE_FORMAT_RGB888_BLUESCREEN:
	case IMAGE_FORMAT_RGB888:            return vk::Format::eR8G8B8Unorm;
	case IMAGE_FORMAT_RGB323232F:        return vk::Format::eR32G32B32Sfloat;

		// BGR
	case IMAGE_FORMAT_BGR888_BLUESCREEN:
	case IMAGE_FORMAT_BGR888:            return vk::Format::eB8G8R8Unorm;
	case IMAGE_FORMAT_BGR565:            return vk::Format::eB5G6R5UnormPack16;

		// RGBA
	case IMAGE_FORMAT_RGBA8888:          return vk::Format::eR8G8B8A8Unorm;
	case IMAGE_FORMAT_RGBA16161616:      return vk::Format::eR16G16B16A16Unorm;
	case IMAGE_FORMAT_RGBA16161616F:     return vk::Format::eR16G16B16A16Sfloat;
	case IMAGE_FORMAT_UVWQ8888:          return vk::Format::eR8G8B8A8Snorm;
	case IMAGE_FORMAT_RGBA32323232F:     return vk::Format::eR32G32B32A32Sfloat;

		// ABGR
	case IMAGE_FORMAT_ARGB8888:
	case IMAGE_FORMAT_ABGR8888:          return vk::Format::eA8B8G8R8UnormPack32;

		// BGRA
	case IMAGE_FORMAT_BGRX8888:
	case IMAGE_FORMAT_BGRA8888:          return vk::Format::eB8G8R8A8Unorm;
	case IMAGE_FORMAT_RGB565:            return vk::Format::eR5G6B5UnormPack16;
	case IMAGE_FORMAT_BGRX5551:
	case IMAGE_FORMAT_BGRA5551:          return vk::Format::eB5G5R5A1UnormPack16;
	case IMAGE_FORMAT_BGRA4444:          return vk::Format::eB4G4R4A4UnormPack16;

		// DXT1
	case IMAGE_FORMAT_DXT1_RUNTIME:
	case IMAGE_FORMAT_DXT1:              return vk::Format::eBc1RgbUnormBlock;
	case IMAGE_FORMAT_DXT1_ONEBITALPHA:  return vk::Format::eBc1RgbaUnormBlock;

		// DXT3
	case IMAGE_FORMAT_DXT3:              return vk::Format::eBc2UnormBlock;

		// DXT5
	case IMAGE_FORMAT_DXT5_RUNTIME:
	case IMAGE_FORMAT_DXT5:              return vk::Format::eBc3UnormBlock;
	}
}

ImageFormat TF2Vulkan::ConvertImageFormat(vk::Format format)
{
	switch (format)
	{
	default: assert(!"Unknown format");
	case vk::Format::eUndefined:            return IMAGE_FORMAT_UNKNOWN;

		// R
	case vk::Format::eR32Sfloat:            return IMAGE_FORMAT_R32F;

		// RG

		// RGB

		// BGR

		// RGBA

		// ABGR

		// BGRA
	case vk::Format::eB8G8R8A8Unorm:        return IMAGE_FORMAT_BGRA8888;

		// DXT1

		// DXT3

		// DXT5
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

	return IMAGE_FORMAT_UNKNOWN;
}

namespace
{
	enum class FilteringSupport
	{
		Optional = false,
		Required = true,

		COUNT,
	};

	using HardwareFormatsArray = std::array<
		std::array<
		std::array<ImageFormat, ImageFormat::NUM_IMAGE_FORMATS>,
		size_t(FormatUsage::COUNT)>,
		size_t(FilteringSupport::COUNT)>;

	static std::once_flag s_HardwareFormatsInitFlag;
	static std::optional<HardwareFormatsArray> s_HardwareFormats;

	static void InitHardwareFormats()
	{
		std::call_once(s_HardwareFormatsInitFlag, []
			{
				s_HardwareFormats.emplace();
				auto& allHWF = *s_HardwareFormats;

				for (size_t filtering = 0; filtering < size_t(FilteringSupport::COUNT); filtering++)
				{
					for (size_t usage = 0; usage < size_t(FormatUsage::COUNT); usage++)
					{
						constexpr auto IMAGE_FORMAT_UNINITIALIZED = ImageFormat(-12345);

						auto & hwf = allHWF[filtering][usage];
						hwf.fill(IMAGE_FORMAT_UNINITIALIZED);

#define PROMOTE_2_HARDWARE(baseFmt, ...) \
	assert(hwf[baseFmt] == IMAGE_FORMAT_UNINITIALIZED); \
	hwf[baseFmt] = FindSupportedFormat(FormatUsage(usage), bool(filtering), { baseFmt, __VA_ARGS__ })

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_ABGR8888,
							IMAGE_FORMAT_ARGB8888,
							IMAGE_FORMAT_BGRA8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ABGR8888,
							IMAGE_FORMAT_RGBA8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGB888,
							IMAGE_FORMAT_BGR888,
							IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_ARGB8888,
							IMAGE_FORMAT_BGRA8888,
							IMAGE_FORMAT_ABGR8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGR888,
							IMAGE_FORMAT_RGB888,
							IMAGE_FORMAT_BGRA8888,
							IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_ABGR8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGB565);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_I8);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_IA88);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_P8);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_A8);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGB888_BLUESCREEN,
							IMAGE_FORMAT_BGR888,
							IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_ARGB8888,
							IMAGE_FORMAT_BGRA8888,
							IMAGE_FORMAT_ABGR8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGR888_BLUESCREEN,
							IMAGE_FORMAT_RGB888,
							IMAGE_FORMAT_BGRA8888,
							IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_ABGR8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ARGB8888,
							IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_BGRA8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGRA8888,
							IMAGE_FORMAT_RGBA8888,
							IMAGE_FORMAT_ARGB8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT1);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT3);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT5);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGRX8888,
							IMAGE_FORMAT_RGBA8888);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGR565);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGRX5551);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGRA4444);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT1_ONEBITALPHA);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_BGRA5551);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_UV88);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_UVWQ8888);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGBA16161616F);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGBA16161616);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_UVLX8888);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_R32F);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGB323232F);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_RGBA32323232F);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_DST16);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_DST24);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_INTZ);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_RAWZ);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI_DST16);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI_DST24);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_NULL);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI2N);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI1N);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT1_RUNTIME);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT5_RUNTIME);

						hwf[IMAGE_FORMAT_P8] = IMAGE_FORMAT_UNKNOWN;

#ifdef _DEBUG
						for (size_t i = 0; i < NUM_IMAGE_FORMATS; i++)
						{
							const auto fmt = ImageFormat(i);
							const auto storedVal = hwf.at(size_t(fmt));
							if (storedVal == IMAGE_FORMAT_UNINITIALIZED)
							{
								Warning(TF2VULKAN_PREFIX "Missing initializer for fmt %zu\n", i);
								assert(false);
							}
						}
#endif
					}
				}
			});
	}
}

ImageFormat TF2Vulkan::PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering)
{
	InitHardwareFormats();

	auto result = s_HardwareFormats->at(size_t(filtering)).at(size_t(usage)).at(size_t(format));

	assert(result != IMAGE_FORMAT_UNKNOWN);

	return result;
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
	return hasSupport;
}
