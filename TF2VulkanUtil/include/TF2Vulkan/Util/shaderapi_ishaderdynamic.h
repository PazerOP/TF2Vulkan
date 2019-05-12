#pragma once

#include "Macros.h"
#include "std_compare.h"

#include <shaderapi/ishaderdynamic.h>

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const LightState_t& lhs, const LightState_t& rhs)
{
	return lhs == rhs ? std::weak_equality::equivalent : std::weak_equality::nonequivalent;
}

inline bool operator<(const ShaderViewport_t& lhs, const ShaderViewport_t& rhs) noexcept
{
	ORDERING_OP_LT_IMPL_PUB(m_nVersion);
	ORDERING_OP_LT_IMPL_PUB(m_nTopLeftX);
	ORDERING_OP_LT_IMPL_PUB(m_nTopLeftY);
	ORDERING_OP_LT_IMPL_PUB(m_nWidth);
	ORDERING_OP_LT_IMPL_PUB(m_nHeight);
	ORDERING_OP_LT_IMPL_PUB(m_flMinZ);
	ORDERING_OP_LT_IMPL_PUB(m_flMaxZ);

	return false; // equal
}

inline std::strong_ordering operator<=>(const ShaderViewport_t& lhs, const ShaderViewport_t& rhs) noexcept
{
	if (lhs < rhs)
		return std::strong_ordering::less;
	else if (lhs == rhs)
		return std::strong_ordering::equal;
	else
		return std::strong_ordering::greater;
}
#endif
