#include "FormatInfo.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"

#include <bitmap/imageformat.h>

#include <mutex>

#undef min
#undef max

using namespace TF2Vulkan;
using namespace TF2Vulkan::FormatInfo;

#define VK_FMT_CASE(base, interpretation) case vk::Format::e ## base ## interpretation :

#define VK_FMT_CASE_ALL(base) \
	VK_FMT_CASE(base, Unorm) \
	VK_FMT_CASE(base, Snorm) \
	VK_FMT_CASE(base, Uscaled) \
	VK_FMT_CASE(base, Sscaled) \
	VK_FMT_CASE(base, Uint) \
	VK_FMT_CASE(base, Sint)

#define VK_FMT_CASE_ALL_SRGB(base) \
	VK_FMT_CASE_ALL(base) \
	VK_FMT_CASE(base, Srgb)

#define VK_FMT_CASE_ALL_FLOAT(base) \
	VK_FMT_CASE_ALL(base) \
	VK_FMT_CASE(base, Sfloat)

vk::Format FormatInfo::ConvertImageFormat(ImageFormat format)
{
	switch (format)
	{
	default: assert(!"Unknown format");
	case IMAGE_FORMAT_UNKNOWN:
	case IMAGE_FORMAT_UVLX8888:
	case IMAGE_FORMAT_P8:
	case IMAGE_FORMAT_NV_INTZ:
	case IMAGE_FORMAT_NV_RAWZ:
	case IMAGE_FORMAT_NV_NULL:
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

		// BC4
	case IMAGE_FORMAT_ATI1N:             return vk::Format::eBc4SnormBlock;
	case IMAGE_FORMAT_ATI2N:             return vk::Format::eBc5SnormBlock;

		// Depth
	case IMAGE_FORMAT_NV_DST16:
	case IMAGE_FORMAT_ATI_DST16:         return vk::Format::eD16Unorm;
	case IMAGE_FORMAT_NV_DST24:
	case IMAGE_FORMAT_ATI_DST24:         return vk::Format::eD24UnormS8Uint;
	}
}

ImageFormat FormatInfo::ConvertImageFormat(vk::Format format)
{
	switch (format)
	{
	default: assert(!"Unknown format");
	case vk::Format::eUndefined:            return IMAGE_FORMAT_UNKNOWN;

		// R
	case vk::Format::eR8Unorm:              return IMAGE_FORMAT_I8;
	case vk::Format::eR32Sfloat:            return IMAGE_FORMAT_R32F;

		// RG

		// RGB

		// BGR

		// RGBA
	case vk::Format::eR8G8B8A8Unorm:        return IMAGE_FORMAT_RGBA8888;
	case vk::Format::eR8G8B8A8Snorm:        return IMAGE_FORMAT_UVWQ8888;
	case vk::Format::eR16G16B16A16Sfloat:   return IMAGE_FORMAT_RGBA16161616F;
	case vk::Format::eR16G16B16A16Unorm:    return IMAGE_FORMAT_RGBA16161616;

		// ABGR
	case vk::Format::eA8B8G8R8UnormPack32:  return IMAGE_FORMAT_ABGR8888;

		// BGRA
	case vk::Format::eB8G8R8A8Unorm:        return IMAGE_FORMAT_BGRA8888;

		// DXT1
	case vk::Format::eBc1RgbaSrgbBlock:
	case vk::Format::eBc1RgbUnormBlock:     return IMAGE_FORMAT_DXT1;

		// DXT3

		// DXT5
	case vk::Format::eBc3UnormBlock:        return IMAGE_FORMAT_DXT5;
	}
}

static ImageFormat FindSupportedFormat(FormatUsage usage, bool filtering, std::initializer_list<ImageFormat> formats)
{
	for (auto fmt : formats)
	{
		if (FormatInfo::HasHardwareSupport(fmt, usage, filtering))
			return fmt;
	}

	return IMAGE_FORMAT_UNKNOWN;
}

static vk::Format FindSupportedFormat(FormatUsage usage, bool filtering, std::initializer_list<vk::Format> formats)
{
	for (auto fmt : formats)
	{
		if (FormatInfo::HasHardwareSupport(fmt, usage, filtering))
			return fmt;
	}

	return vk::Format::eUndefined;
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

#pragma push_macro("PROMOTE_2_HARDWARE")
#undef PROMOTE_2_HARDWARE
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

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_DST16,
							IMAGE_FORMAT_ATI_DST16,
							IMAGE_FORMAT_NV_DST24,
							IMAGE_FORMAT_ATI_DST24);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_DST24,
							IMAGE_FORMAT_ATI_DST24,
							IMAGE_FORMAT_NV_DST16,
							IMAGE_FORMAT_ATI_DST16);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_INTZ);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_RAWZ);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI_DST16,
							IMAGE_FORMAT_NV_DST16,
							IMAGE_FORMAT_ATI_DST24,
							IMAGE_FORMAT_NV_DST24);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI_DST24,
							IMAGE_FORMAT_NV_DST24,
							IMAGE_FORMAT_ATI_DST16,
							IMAGE_FORMAT_NV_DST16);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_NV_NULL);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI2N);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_ATI1N);

						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT1_RUNTIME);
						PROMOTE_2_HARDWARE(IMAGE_FORMAT_DXT5_RUNTIME);

						hwf[IMAGE_FORMAT_P8] = IMAGE_FORMAT_UNKNOWN;

#pragma pop_macro("PROMOTE_2_HARDWARE")

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

ImageFormat FormatInfo::PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering)
{
	InitHardwareFormats();

	auto result = s_HardwareFormats->at(size_t(filtering)).at(size_t(usage)).at(size_t(format));

	assert(result != IMAGE_FORMAT_UNKNOWN);

	return result;
}

vk::Format FormatInfo::PromoteToHardware(vk::Format format, FormatUsage usage, bool filtering)
{
	switch (format)
	{
	default:
		Warning(TF2VULKAN_PREFIX "No fallback formats defined for %s\n", vk::to_string(format).c_str());
		if (HasHardwareSupport(format, usage, filtering))
			return format;
		else
			return vk::Format::eUndefined;

#pragma push_macro("PROMOTE_2_HARDWARE")
#undef PROMOTE_2_HARDWARE
#define PROMOTE_2_HARDWARE(fmt, ...) \
	case fmt : return FindSupportedFormat(usage, filtering, { fmt, __VA_ARGS__ })

		PROMOTE_2_HARDWARE(vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD32SfloatS8Uint);

#pragma pop_macro("PROMOTE_2_HARDWARE")
	}
}

vk::Extent2D FormatInfo::GetBlockSize(vk::Format format)
{
	switch (format)
	{
	case vk::Format::eBc1RgbSrgbBlock:
	case vk::Format::eBc1RgbUnormBlock:

	case vk::Format::eBc1RgbaSrgbBlock:
	case vk::Format::eBc1RgbaUnormBlock:

	case vk::Format::eBc3SrgbBlock:
	case vk::Format::eBc3UnormBlock:
		return vk::Extent2D(4, 4);

	default:
		assert(!"Unknown format");

		VK_FMT_CASE_ALL_SRGB(R8);
		VK_FMT_CASE_ALL_SRGB(R8G8);
		VK_FMT_CASE_ALL_SRGB(R8G8B8);
		VK_FMT_CASE_ALL_SRGB(R8G8B8A8);
		VK_FMT_CASE_ALL_SRGB(B8G8R8);
		VK_FMT_CASE_ALL_SRGB(B8G8R8A8);
		VK_FMT_CASE_ALL_FLOAT(R16G16B16A16);

	case vk::Format::eD24UnormS8Uint:
		return vk::Extent2D(1, 1);
	}
}

size_t FormatInfo::GetPixelSize(vk::Format format)
{
	switch (format)
	{
		VK_FMT_CASE_ALL_SRGB(R8)
			return 1;

		VK_FMT_CASE_ALL_SRGB(R8G8)
			return 2;

		VK_FMT_CASE_ALL_SRGB(B8G8R8)
		VK_FMT_CASE_ALL_SRGB(R8G8B8)
			return 3;

		VK_FMT_CASE_ALL_SRGB(B8G8R8A8)
		VK_FMT_CASE_ALL_SRGB(R8G8B8A8)
			return 4;

		VK_FMT_CASE_ALL_FLOAT(R16G16B16A16)
			return 8;
	}

	assert(!"Unknown/unsupported format");
	return 0;
}

uint32_t FormatInfo::GetStride(vk::Format format, uint32_t width)
{
	return GetPixelSize(format) * width;
}

vk::Extent3D FormatInfo::GetMipResolution(uint_fast8_t mipIndex,
	uint32_t inWidth, uint32_t inHeight, uint32_t inDepth)
{
	return
	{
		std::max<uint32_t>(inWidth >> mipIndex, 1),
		std::max<uint32_t>(inHeight >> mipIndex, 1),
		std::max<uint32_t>(inDepth >> mipIndex, 1),
	};
}

size_t FormatInfo::GetSliceSize(vk::Format format, uint32_t width, uint32_t height)
{
	return GetStride(format, width) * height;
}

size_t FormatInfo::GetFrameSize3D(vk::Format format,
	uint32_t width, uint32_t height, uint32_t depth,
	uint_fast8_t mipCount, uint_fast8_t firstMip)
{
	size_t retVal = 0;

	for (uint_fast8_t i = 0; i < mipCount; i++)
	{
		const auto [mipW, mipH, mipD] = GetMipResolution(firstMip + i, width, height, depth);
		retVal += GetSliceSize(format, mipW, mipH) * mipD;
	}

	return retVal;
}

FormatInfo::SubrectOffset FormatInfo::GetSubrectOffset(vk::Format format,
	uint32_t offsetX, uint32_t offsetY, uint32_t offsetZ,
	uint32_t width, uint32_t height, uint32_t depth,
	uint_fast8_t mipLevel)
{
	size_t offset = GetFrameSize3D(format, width, height, depth, mipLevel);

	const auto [mipW, mipH, mipD] = GetMipResolution(mipLevel, width, height, depth);
	const uint32_t pixelSize = FormatInfo::GetPixelSize(format);
	const uint32_t mipStride = FormatInfo::GetStride(format, width);
	const uint32_t mipSliceStride = FormatInfo::GetSliceSize(format, mipW, mipH);

	offset += (mipSliceStride * offsetZ) + (mipStride * offsetY) + (pixelSize * offsetX);
	return { offset, mipStride };
}

vk::ImageAspectFlags FormatInfo::GetAspects(const vk::Format& format)
{
	switch (format)
	{
	case vk::Format::eR4G4UnormPack8:
	case vk::Format::eR4G4B4A4UnormPack16:
	case vk::Format::eB4G4R4A4UnormPack16:
	case vk::Format::eR5G6B5UnormPack16:
	case vk::Format::eB5G6R5UnormPack16:
	case vk::Format::eR5G5B5A1UnormPack16:
	case vk::Format::eB5G5R5A1UnormPack16:
	case vk::Format::eA1R5G5B5UnormPack16:

		VK_FMT_CASE_ALL_SRGB(R8);
		VK_FMT_CASE_ALL_SRGB(R8G8);
		VK_FMT_CASE_ALL_SRGB(R8G8B8);
		VK_FMT_CASE_ALL_SRGB(R8G8B8A8);
		VK_FMT_CASE_ALL_SRGB(B8G8R8);
		VK_FMT_CASE_ALL_SRGB(B8G8R8A8);

	case vk::Format::eA8B8G8R8UnormPack32:
	case vk::Format::eA8B8G8R8SnormPack32:
	case vk::Format::eA8B8G8R8UscaledPack32:
	case vk::Format::eA8B8G8R8SscaledPack32:
	case vk::Format::eA8B8G8R8UintPack32:
	case vk::Format::eA8B8G8R8SintPack32:
	case vk::Format::eA8B8G8R8SrgbPack32:

		VK_FMT_CASE_ALL_FLOAT(R16G16B16A16);

	case vk::Format::eBc1RgbUnormBlock:
	case vk::Format::eBc1RgbSrgbBlock:
	case vk::Format::eBc1RgbaUnormBlock:
	case vk::Format::eBc1RgbaSrgbBlock:

	case vk::Format::eBc3UnormBlock:
	case vk::Format::eBc3SrgbBlock:
		return vk::ImageAspectFlagBits::eColor;

	case vk::Format::eD24UnormS8Uint:
	case vk::Format::eD16UnormS8Uint:
	case vk::Format::eD32SfloatS8Uint:
		return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	}

	assert(!"Unknown/unsupported format");
	return vk::ImageAspectFlags{};
}

static constexpr uint_fast32_t PackDataFormat(DataFormat fmt,
	uint_fast8_t components, uint_fast8_t componentSize)
{
	return (uint_fast32_t(fmt) << 16) | (uint_fast32_t(componentSize) << 8) | (uint_fast32_t(components));
}
template<DataFormat fmt, uint_fast8_t components, uint_fast8_t componentSize>
static constexpr auto PACK_DFMT = PackDataFormat(fmt, components, componentSize);

vk::Format FormatInfo::ConvertDataFormat(DataFormat fmt, uint_fast8_t components, uint_fast8_t componentSize)
{
	switch (PackDataFormat(fmt, components, componentSize))
	{
	case PACK_DFMT<DataFormat::UInt, 1, 1>:          return vk::Format::eR8Uint;
	case PACK_DFMT<DataFormat::UInt, 2, 1>:          return vk::Format::eR8G8Uint;
	case PACK_DFMT<DataFormat::UInt, 3, 1>:          return vk::Format::eR8G8B8Uint;
	case PACK_DFMT<DataFormat::UInt, 4, 1>:          return vk::Format::eR8G8B8A8Uint;

	case PACK_DFMT<DataFormat::UIntCastFloat, 1, 1>: return vk::Format::eR8Uscaled;
	case PACK_DFMT<DataFormat::UIntCastFloat, 2, 1>: return vk::Format::eR8G8Uscaled;
	case PACK_DFMT<DataFormat::UIntCastFloat, 3, 1>: return vk::Format::eR8G8B8Uscaled;
	case PACK_DFMT<DataFormat::UIntCastFloat, 4, 1>: return vk::Format::eR8G8B8A8Uscaled;

	case PACK_DFMT<DataFormat::SIntCastFloat, 1, 1>: return vk::Format::eR8Sscaled;
	case PACK_DFMT<DataFormat::SIntCastFloat, 2, 1>: return vk::Format::eR8G8Sscaled;
	case PACK_DFMT<DataFormat::SIntCastFloat, 3, 1>: return vk::Format::eR8G8B8Sscaled;
	case PACK_DFMT<DataFormat::SIntCastFloat, 4, 1>: return vk::Format::eR8G8B8A8Sscaled;

	case PACK_DFMT<DataFormat::SFloat, 1, 2>:        return vk::Format::eR16Sfloat;
	case PACK_DFMT<DataFormat::SFloat, 2, 2>:        return vk::Format::eR16G16Sfloat;
	case PACK_DFMT<DataFormat::SFloat, 3, 2>:        return vk::Format::eR16G16B16Sfloat;
	case PACK_DFMT<DataFormat::SFloat, 4, 2>:        return vk::Format::eR16G16B16A16Sfloat;

	case PACK_DFMT<DataFormat::SFloat, 1, 4>:        return vk::Format::eR32Sfloat;
	case PACK_DFMT<DataFormat::SFloat, 2, 4>:        return vk::Format::eR32G32Sfloat;
	case PACK_DFMT<DataFormat::SFloat, 3, 4>:        return vk::Format::eR32G32B32Sfloat;
	case PACK_DFMT<DataFormat::SFloat, 4, 4>:        return vk::Format::eR32G32B32A32Sfloat;

	case PACK_DFMT<DataFormat::UNorm, 1, 1>:         return vk::Format::eR8Unorm;
	case PACK_DFMT<DataFormat::UNorm, 2, 1>:         return vk::Format::eR8G8Unorm;
	case PACK_DFMT<DataFormat::UNorm, 3, 1>:         return vk::Format::eR8G8B8Unorm;
	case PACK_DFMT<DataFormat::UNorm, 4, 1>:         return vk::Format::eR8G8B8A8Unorm;
	}

	assert(!"Unknown/unsupported combination");
	return vk::Format::eUndefined;
}

bool FormatInfo::IsCompressed(vk::Format format)
{
	switch (format)
	{
	default:
		assert(!"Unknown/unsupported format");

	case vk::Format::eUndefined:

		VK_FMT_CASE_ALL_SRGB(R8G8B8A8);
		VK_FMT_CASE_ALL_SRGB(B8G8R8A8);

		return false;

	case vk::Format::eBc1RgbaSrgbBlock:
	case vk::Format::eBc1RgbaUnormBlock:
	case vk::Format::eBc1RgbSrgbBlock:
	case vk::Format::eBc1RgbUnormBlock:
		return true;
	}
}

bool FormatInfo::HasHardwareSupport(ImageFormat format, FormatUsage usage, bool filtering)
{
	auto vkFormat = ConvertImageFormat(format);
	if (vkFormat == vk::Format::eUndefined)
		return false;

	return HasHardwareSupport(vkFormat, usage, filtering);
}

bool FormatInfo::HasHardwareSupport(vk::Format format, FormatUsage usage, bool filtering)
{
	auto formatProperties = g_ShaderDeviceMgr.GetAdapter().getFormatProperties(format);

	using Flags = vk::FormatFeatureFlagBits;

	vk::FormatFeatureFlags REQUIRED_FLAGS{};

	if (usage == FormatUsage::ImmutableTexture ||
		usage == FormatUsage::RenderTarget ||
		usage == FormatUsage::DepthStencil ||
		usage == FormatUsage::ReadBackTexture)
	{
		if (filtering)
			REQUIRED_FLAGS |= Flags::eSampledImageFilterLinear;
		else
			REQUIRED_FLAGS |= Flags::eSampledImage;

		if (usage == FormatUsage::RenderTarget)
			REQUIRED_FLAGS |= Flags::eColorAttachment;
		else if (usage == FormatUsage::DepthStencil)
			REQUIRED_FLAGS |= Flags::eDepthStencilAttachment;

		if (usage == FormatUsage::ReadBackTexture)
			REQUIRED_FLAGS |= Flags::eTransferDst;
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

#if false
#ifdef _DEBUG
namespace
{
	struct BadUnitTesting final
	{
		BadUnitTesting()
		{
			for (int i = VK_FORMAT_BEGIN_RANGE; i <= VK_FORMAT_END_RANGE; i++)
			{
				const auto fmt = (vk::Format)i;

				if (fmt == vk::Format::eUndefined)
					continue;

				GetAspects(fmt);
				GetBlockSize(fmt);
				IsCompressed(fmt);
			}
		}

	} s_BadUnitTesting;
}
#endif
#endif
