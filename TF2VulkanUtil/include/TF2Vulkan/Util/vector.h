#pragma once

#include "std_compare.h"

#include <mathlib/vector.h>

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const Vector& lhs, const Vector& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
#endif
