#pragma once

#include "ConstexprCRC.h"
#include "ConstexprRandom.h"

#include <Color.h>

#if _MSC_VER <= 1920
// https://developercommunity.visualstudio.com/content/problem/211134/unsigned-integer-overflows-in-constexpr-functionsa.html
#pragma warning(disable : 4307)
#endif

namespace Util
{
	template<bool randomAlpha = false, size_t fileNameLength = 0>
	constexpr Color RandomColor(const char(&fileName)[fileNameLength], size_t line, size_t customOffset = 0)
	{
		CRC32_t crc = CRC32::INIT_VALUE;
		CRC32::ProcessBuffer(crc, fileName);
		CRC32::ProcessValue(crc, line);
		CRC32::ProcessValue(crc, customOffset);
		CRC32::Finalize(crc);

		int r = crand(crc, 0, 256);
		int g = crand(crc, 0, 256);
		int b = crand(crc, 0, 256);
		int a = randomAlpha ? crand(crc, 0, 256) : 255;
		return Color(r, g, b, a);
	}
}

#define TF2VULKAN_RANDOM_COLOR_FROM_LOCATION(...) ::Util::RandomColor(__FILE__, __LINE__, __VA_ARGS__)
