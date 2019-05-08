#pragma once

#include "std_compare.h"

#include <mathlib/vector2d.h>
#include <mathlib/vector.h>
#include <mathlib/vector4d.h>

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const Vector2D& lhs, const Vector2D& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
inline std::weak_equality operator<=>(const Vector& lhs, const Vector& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
inline std::weak_equality operator<=>(const Vector4D& lhs, const Vector4D& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
#endif
