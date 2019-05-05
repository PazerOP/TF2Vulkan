#pragma once

enum ImageFormat;

namespace TF2Vulkan
{
	vk::Format ConvertImageFormat(ImageFormat format);
	ImageFormat ConvertImageFormat(vk::Format format);

	enum class FormatUsage
	{
		ImmutableTexture,
		RenderTarget,
		DepthStencil,

		COUNT,
	};

	bool HasHardwareSupport(ImageFormat format, FormatUsage usage, bool filtering);
	bool HasHardwareSupport(vk::Format format, FormatUsage usage, bool filtering);
	ImageFormat PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering);
	vk::Format PromoteToHardware(vk::Format format, FormatUsage usage, bool filtering);
	vk::Extent2D GetBlockSize(vk::Format format);

	vk::ImageAspectFlags GetAspects(const vk::Format& format);

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

	vk::Format ConvertDataFormat(DataFormat fmt, uint_fast8_t components, uint_fast8_t componentSize);
	[[nodiscard]] bool ConvertDataFormat(vk::Format inFmt, DataFormat& outFmt, uint_fast8_t& numComponents, uint_fast8_t& byteSize);
}
