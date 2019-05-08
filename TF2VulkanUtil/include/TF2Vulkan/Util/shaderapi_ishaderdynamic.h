#pragma once

#include <shaderapi/ishaderdynamic.h>
#include "std_compare.h"

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const LightState_t& lhs, const LightState_t& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}
#endif
