#pragma once

#include <cmath>

#undef min
#undef max

using namespace TF2Vulkan::Shaders;

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr inline T distance(const T& a, const T& b)
{
	return (a > b) ? (a - b) : (b - a);
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr inline T dst(const T& a, const T& b)
{
	return distance(a, b);
}

template<typename T, size_t elements>
inline vector<T, elements> dst(const vector<T, elements>& lhs, const vector<T, elements>& rhs)
{
	vector<T, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = distance(lhs[i], rhs[i]);

	return retVal;
}

constexpr inline float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

inline float rsqrt(float x)
{
	return 1.0f / sqrt(x);
}

template<typename T, size_t elements>
inline T dot(const vector<T, elements>& lhs, const vector<T, elements>& rhs)
{
	T retVal{};

	for (size_t i = 0; i < elements; i++)
		retVal += lhs[i] * rhs[i];

	return retVal;
}

template<typename TElem, size_t elements, typename TScalar>
inline vector<TElem, elements> lerp(const vector<TElem, elements>& lhs, const vector<TElem, elements>& rhs, const TScalar& scalar)
{
	vector<TElem, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = lerp(lhs[i], rhs[i], scalar);

	return retVal;
}

template<typename T>
constexpr inline T max(const T& a, const T& b)
{
	return (a > b) ? a : b;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr inline T clamp(const T& x, const T& min, const T& max)
{
	if (x < min)
		return min;

	if (x > max)
		return max;

	return max;
}

template<typename T, size_t elements>
constexpr inline vector<T, elements> clamp(const vector<T, elements>& x, const vector<T, elements>& min, const vector<T, elements>& max)
{
	vector<T, elements> retVal;

	for (size_t i = 0; i < elements; i++)
		retVal[i] = clamp(x[i], min[i], max[i]);

	return retVal;
}

template<typename T, size_t elements>
constexpr inline vector<T, elements> saturate(const vector<T, elements>& x)
{
	return clamp(x, vector<T, elements>(0), vector<T, elements>(1));
}

template<typename T>
constexpr inline T saturate(const T& x)
{
	return clamp(x, (T)0, (T)1);
}

template<typename T, size_t elements>
inline T length(const vector<T, elements>& x)
{
	return sqrt(dot(x, x));
}

template<typename T, size_t elements>
inline vector<T, elements> normalize(const vector<T, elements>& x)
{
	return x / length(x);
}
