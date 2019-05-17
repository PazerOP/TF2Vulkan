#pragma once

#include <TF2Vulkan/AlignedTypes.h>

namespace TF2Vulkan{ namespace Shaders
{
	struct TextureTransform final
	{
		TextureTransform() = default;
		explicit TextureTransform(const VMatrix& matrix)
		{
			for (uint_fast8_t y = 0; y < 2; y++)
			{
				for (uint_fast8_t x = 0; x < 4; x++)
					m_PackedMatrix[y][x] = matrix[y][x];
			}
		}

		float4x2 m_PackedMatrix;
	};
} }
