#pragma once

#include "std_compare.h"
#include <mathlib/lightdesc.h>

inline bool operator==(const LightDesc_t& lhs, const LightDesc_t& rhs)
{
	return
		lhs.m_Type == rhs.m_Type &&
		lhs.m_Color == rhs.m_Color &&
		lhs.m_Position == rhs.m_Position &&
		lhs.m_Direction == rhs.m_Direction &&
		lhs.m_Range == rhs.m_Range &&
		lhs.m_Falloff == rhs.m_Falloff &&
		lhs.m_Attenuation0 == rhs.m_Attenuation0 &&
		lhs.m_Attenuation1 == rhs.m_Attenuation1 &&
		lhs.m_Attenuation2 == rhs.m_Attenuation2 &&
		lhs.m_Theta == rhs.m_Theta &&
		lhs.m_Phi == rhs.m_Phi &&
		lhs.m_ThetaDot == rhs.m_ThetaDot &&
		lhs.m_PhiDot == rhs.m_PhiDot &&
		lhs.m_Flags == rhs.m_Flags;
}

inline bool operator!=(const LightDesc_t& lhs, const LightDesc_t& rhs)
{
	return !operator==(lhs, rhs);
}

#ifndef __INTELLISENSE__
inline std::weak_equality operator<=>(const LightDesc_t& lhs, const LightDesc_t& rhs)
{
	return lhs == rhs ? std::strong_equality::equal : std::strong_equality::nonequal;
}
#endif
