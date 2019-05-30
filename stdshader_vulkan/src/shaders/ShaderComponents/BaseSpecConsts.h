#pragma once

#include "TF2Vulkan/AlignedTypes.h"

namespace TF2Vulkan{ namespace Shaders
{
#define SPEC_CONST_LIST \
	SC(VERTEXCOLOR, bool32) SEP \
	SC(SKINNING, boo32) SEP

	enum class SpecConstType
	{
		VERTEXCOLOR,
	};

	template<SpecConstType type> struct SpecConstTypeDecl;

	template<SpecConstType type> struct BaseSpecConstBuf;

	template<> struct BaseSpecConstBuf<SpecConstType::VERTEXCOLOR> { bool32 VERTEXCOLOR; };
} }
