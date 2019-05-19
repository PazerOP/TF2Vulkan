#pragma once

namespace Util
{
	struct DefaultCRandConstants
	{
		static constexpr size_t m = (1UL << 31);
		static constexpr size_t a = 1103515245;
		static constexpr size_t c = 12345;
	};

	// See https://stackoverflow.com/a/3062783/871842 and
	// https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
	// These parameters are from glibc
	template<typename T = DefaultCRandConstants>
	constexpr size_t crand(size_t& seed)
	{
		seed = (T::a * seed + T::c) % T::m;
		return seed;
	}

	template<typename T = DefaultCRandConstants>
	constexpr size_t crand(size_t& seed, size_t min, size_t max)
	{
		return crand<T>(seed) % (max - min) + min;
	}

	template<typename T = DefaultCRandConstants>
	constexpr size_t crand_cseed(size_t seed, size_t min, size_t max)
	{
		return crand<T>(seed, min, max);
	}
}
