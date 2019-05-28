#pragma once

#include "TextureSubrect.h"

namespace TF2Vulkan
{
	struct TextureData : TextureSubrect
	{
		constexpr TextureData() = default;
		constexpr TextureData(const TextureSubrect& baseRect) : TextureSubrect(baseRect) {}

		ImageFormat m_Format = ImageFormat::IMAGE_FORMAT_UNKNOWN;

		void* m_Data = nullptr;
		size_t m_DataLength = 0;
		size_t m_Stride = 0;
		size_t m_SliceStride = 0;

		bool Validate() const;
		bool ValidateNoAssert() const;
	};
}
