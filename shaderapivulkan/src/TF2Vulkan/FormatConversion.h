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
}
