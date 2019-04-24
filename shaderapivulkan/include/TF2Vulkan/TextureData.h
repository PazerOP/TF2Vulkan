#pragma once

#include <TF2Vulkan/Util/ImageManip.h>

#include <bitmap/imageformat.h>
#include <vtf/vtf.h>

namespace TF2Vulkan
{
	struct TextureData
	{
		uint_fast8_t m_MipLevel = uint_fast8_t(-1);
		CubeMapFaceIndex_t m_CubeFace{};
		uint_fast32_t m_Width = 0;
		uint_fast32_t m_Height = 0;
		uint_fast32_t m_Depth = 1;

		uint_fast32_t m_XOffset = 0;
		uint_fast32_t m_YOffset = 0;
		uint_fast32_t m_ZOffset = 0;

		ImageFormat m_Format = ImageFormat::IMAGE_FORMAT_UNKNOWN;
		const void* m_Data = nullptr;
		size_t m_DataLength = 0;
		size_t m_Stride = 0;
		size_t m_SliceStride = 0;

		bool Validate() const;
		bool ValidateNoAssert() const;
	};
}
