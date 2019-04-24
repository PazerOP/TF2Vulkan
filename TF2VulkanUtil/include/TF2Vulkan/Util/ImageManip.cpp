#include "ImageManip.h"

#include <algorithm>

using namespace ImageManip;

IntVec2D ImageManip::ComputeMipResolution(const IntVec2D& baseSize, uint_fast8_t mipLevel)
{
	return IntVec2D(
		ComputeMipResolution(baseSize.x, mipLevel),
		ComputeMipResolution(baseSize.y, mipLevel));
}

IntVec3D ImageManip::ComputeMipResolution(const IntVec3D& baseSize, uint_fast8_t mipLevel)
{
	return IntVec3D(
		ComputeMipResolution(baseSize.x, mipLevel),
		ComputeMipResolution(baseSize.y, mipLevel),
		ComputeMipResolution(baseSize.z, mipLevel));
}
