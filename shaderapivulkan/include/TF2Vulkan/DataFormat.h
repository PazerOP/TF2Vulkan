#pragma once

#include <cstdint>

namespace TF2Vulkan
{
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
}
