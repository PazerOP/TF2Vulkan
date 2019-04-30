#include "stdafx.h"
#include "GraphicsPipeline.h"

using namespace TF2Vulkan;

bool PixelShaderStageSettings::operator==(const PixelShaderStageSettings& other) const noexcept
{
	return
		*static_cast<const ShaderStageSettings*>(this) == other &&
		m_Samplers == other.m_Samplers;
}
