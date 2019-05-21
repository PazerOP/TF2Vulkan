#pragma once

#include <TF2Vulkan/DataFormat.h>

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
}

namespace TF2Vulkan{ namespace FormatInfo
{
	vk::Format ConvertImageFormat(ImageFormat format);
	ImageFormat ConvertImageFormat(vk::Format format);

	bool IsCompressed(vk::Format format);
	bool HasHardwareSupport(vk::Format format, FormatUsage usage, bool filtering);
	bool HasHardwareSupport(ImageFormat format, FormatUsage usage, bool filtering);
	ImageFormat PromoteToHardware(ImageFormat format, FormatUsage usage, bool filtering);
	vk::Format PromoteToHardware(vk::Format format, FormatUsage usage, bool filtering);
	vk::Extent2D GetBlockSize(vk::Format format);
	size_t GetPixelSize(vk::Format format);
	size_t GetStride(vk::Format format, uint32_t width);
	vk::Extent3D GetMipResolution(uint_fast8_t mipIndex, uint32_t width, uint32_t height, uint32_t depth = 1);
	size_t GetSliceSize(vk::Format format, uint32_t width, uint32_t height);

	size_t GetFrameSize3D(vk::Format format,
		uint32_t width, uint32_t height, uint32_t depth,
		uint_fast8_t mipCount = 1, uint_fast8_t firstMip = 0);

	struct SubrectOffset
	{
		size_t m_Offset;
		size_t m_Stride;
	};
	SubrectOffset GetSubrectOffset(vk::Format format,
		uint32_t offsetX, uint32_t offsetY, uint32_t offsetZ,
		uint32_t width, uint32_t height, uint32_t depth,
		uint_fast8_t mipLevel = 0);

	vk::ImageAspectFlags GetAspects(const vk::Format& format);

	vk::Format ConvertDataFormat(DataFormat fmt, uint_fast8_t components, uint_fast8_t componentSize);
	[[nodiscard]] bool ConvertDataFormat(vk::Format inFmt, DataFormat& outFmt, uint_fast8_t& numComponents, uint_fast8_t& byteSize);

	inline size_t GetFrameSize2D(vk::Format format,
		uint32_t width, uint32_t height,
		uint_fast8_t mipCount = 1, uint_fast8_t firstMip = 0)
	{
		return GetFrameSize3D(format, width, height, 1, mipCount, firstMip);
	}
	inline vk::Extent2D GetBlockSize(ImageFormat format)
	{
		return GetBlockSize(ConvertImageFormat(format));
	}
	inline bool IsCompressed(ImageFormat format)
	{
		return IsCompressed(ConvertImageFormat(format));
	}
	inline size_t GetStride(ImageFormat format, uint32_t width)
	{
		return GetStride(ConvertImageFormat(format), width);
	}
	inline size_t GetPixelSize(ImageFormat format)
	{
		return GetPixelSize(ConvertImageFormat(format));
	}
	inline size_t GetSliceSize(ImageFormat format, uint32_t width, uint32_t height)
	{
		return GetSliceSize(ConvertImageFormat(format), width, height);
	}
} }
