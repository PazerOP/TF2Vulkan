#pragma once

#include <bitmap/imageformat.h>
#include <vtf/vtf.h>

namespace TF2Vulkan
{
	struct TextureSubrect
	{
		uint_fast8_t m_MipLevel = uint_fast8_t(-1);
		CubeMapFaceIndex_t m_CubeFace{};
		uint_fast32_t m_Width = 0;
		uint_fast32_t m_Height = 0;
		uint_fast32_t m_Depth = 1;

		uint_fast32_t m_OffsetX = 0;
		uint_fast32_t m_OffsetY = 0;
		uint_fast32_t m_OffsetZ = 0;
	};
}
