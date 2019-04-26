#include "TF2Vulkan/Util/std_utility.h"

size_t Util::hash_combine(size_t h1, size_t h2)
{
	// This line of code is from boost 1.55:
	// https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
	// See https://www.boost.org/LICENSE_1_0.txt
	return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
}
