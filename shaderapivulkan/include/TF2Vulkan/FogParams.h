#pragma once

#include "TF2Vulkan/Util/std_compare.h"

namespace TF2Vulkan
{
	struct LogicalFogParams final
	{
		constexpr LogicalFogParams() = default;

		DEFAULT_PARTIAL_ORDERING_OPERATOR(LogicalFogParams);

		float m_Start = 0;
		float m_End = 0;
		float m_Z = 0;
		float m_MaxDensity = 0;
		MaterialFogMode_t m_Mode = MATERIAL_FOG_NONE;
		uint8_t m_ColorR = 255;
		uint8_t m_ColorG = 255;
		uint8_t m_ColorB = 255;
	};
}
