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

		COUNT,
	};

	bool HasHardwareSupport(ImageFormat format, FormatUsage usage, bool filtering);
	ImageFormat PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering);

	enum class DataFormat : uint_fast8_t
	{
		Invalid = uint_fast8_t(-1),

		UNorm = 0,
		SNorm,
		UInt,
		SInt,
		UFloat,
		SFloat,
	};

	vk::Format ConvertDataFormat(DataFormat fmt, uint_fast8_t components, uint_fast8_t componentSize);
	[[nodiscard]] bool ConvertDataFormat(vk::Format inFmt, DataFormat& outFmt, uint_fast8_t& numComponents, uint_fast8_t& byteSize);
}
