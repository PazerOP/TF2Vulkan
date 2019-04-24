#pragma once

#include <cstdint>

namespace vk
{
	struct Extent3D;
}

namespace ImageManip
{
	using Dim = uint32_t;
	union IntVec2D
	{
		IntVec2D() = default;
		explicit constexpr IntVec2D(Dim dim) :
			x(dim), y(dim)
		{
		}
		constexpr IntVec2D(Dim x_, Dim y_) :
			x(x_), y(y_)
		{
		}

		struct
		{
			Dim x;
			Dim y;
		};
		Dim values[2];
	};

	union IntVec3D
	{
		IntVec3D() = default;
		explicit IntVec3D(const vk::Extent3D& extent);
		explicit constexpr IntVec3D(Dim dim) :
			x(dim), y(dim), z(dim)
		{
		}
		constexpr IntVec3D(Dim x_, Dim y_, Dim z_) :
			x(x_), y(y_), z(z_)
		{
		}

		struct
		{
			Dim x;
			Dim y;
			Dim z;
		};
		Dim values[3];
	};

	template<typename T> inline T ComputeMipResolution(T baseSize, uint_fast8_t mipLevel)
	{
		baseSize >>= mipLevel;
		if (baseSize < 0)
			baseSize = 1;

		return baseSize;
	}
	IntVec2D ComputeMipResolution(const IntVec2D& baseSize, uint_fast8_t mipLevel);
	IntVec3D ComputeMipResolution(const IntVec3D& baseSize, uint_fast8_t mipLevel);
}
