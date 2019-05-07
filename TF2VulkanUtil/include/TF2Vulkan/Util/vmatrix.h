#pragma once

#include <mathlib/vmatrix.h>

#include "std_compare.h"

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const VMatrix& lhs, const VMatrix& rhs)
{
	return (lhs == rhs) ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
#endif
