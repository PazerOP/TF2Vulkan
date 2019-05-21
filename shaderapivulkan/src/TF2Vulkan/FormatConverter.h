#pragma once

#include <bitmap/imageformat.h>

enum ImageFormat;

namespace TF2Vulkan{ namespace FormatConverter
{
	void Convert(
		const std::byte* src, ImageFormat srcFormat, size_t srcSize,
		std::byte* dst, ImageFormat dstFormat, size_t dstSize,
		uint32_t width, uint32_t height,
		size_t srcStride = 0, size_t dstStride = 0);

	template<ImageFormat format> struct ImageFormatType final
	{
		static constexpr auto FORMAT = format;
		constexpr operator ImageFormat() const { return format; }
	};

	template<typename TFunc, typename... TArgs>
	static constexpr auto ImageFormatVisit(ImageFormat fmt, const TFunc& func, TArgs&& ... args)
	{
#pragma push_macro("IFV_CASE")
#undef IFV_CASE
#define IFV_CASE(fmt) case fmt : return func(ImageFormatType<fmt>{}, std::forward<TArgs>(args)...)

		switch (fmt)
		{
			IFV_CASE(IMAGE_FORMAT_RGBA8888);
			IFV_CASE(IMAGE_FORMAT_ABGR8888);
			IFV_CASE(IMAGE_FORMAT_RGB888);
			IFV_CASE(IMAGE_FORMAT_BGR888);
			IFV_CASE(IMAGE_FORMAT_ARGB8888);
			IFV_CASE(IMAGE_FORMAT_BGRA8888);
			IFV_CASE(IMAGE_FORMAT_BGRX8888);

		default:
			throw VulkanException("Unexpected/unsupported imageformat", EXCEPTION_DATA());
		}

#pragma pop_macro("IFV_CASE")
	}
} }
