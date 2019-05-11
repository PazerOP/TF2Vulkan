#pragma once

#include <cstdint>

enum ImageFormat;

namespace TF2Vulkan
{
	enum class FormatUsage
	{
		ImmutableTexture,
		RenderTarget,
		DepthStencil,

		COUNT,
	};

	enum class DataFormat : uint_fast8_t
	{
		Invalid = uint_fast8_t(-1),

		UNorm = 0,
		SNorm,
		UIntCastFloat,
		SIntCastFloat,
		UInt,
		SInt,
		UFloat,
		SFloat,
	};
}

namespace TF2Vulkan{ namespace FormatInfo
{
	vk::Format ConvertImageFormat(ImageFormat format);
	ImageFormat ConvertImageFormat(vk::Format format);

	bool IsCompressed(ImageFormat format);
	bool IsCompressed(vk::Format format);
	bool HasHardwareSupport(ImageFormat format, FormatUsage usage, bool filtering);
	bool HasHardwareSupport(vk::Format format, FormatUsage usage, bool filtering);
	ImageFormat PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering);
	vk::Format PromoteToHardware(vk::Format format, FormatUsage usage, bool filtering);
	vk::Extent2D GetBlockSize(ImageFormat format);
	vk::Extent2D GetBlockSize(vk::Format format);
	size_t GetPixelSize(ImageFormat format);
	size_t GetPixelSize(vk::Format format);

	vk::ImageAspectFlags GetAspects(const vk::Format& format);

	vk::Format ConvertDataFormat(DataFormat fmt, uint_fast8_t components, uint_fast8_t componentSize);
	[[nodiscard]] bool ConvertDataFormat(vk::Format inFmt, DataFormat& outFmt, uint_fast8_t& numComponents, uint_fast8_t& byteSize);
} }
