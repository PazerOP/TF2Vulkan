#include "stdafx.h"
#include "GraphicsPipeline.h"

using namespace TF2Vulkan;

bool PixelShaderStageSettings::operator==(const PixelShaderStageSettings& rhs) const noexcept
{
	return
		static_cast<const ShaderStageSettings&>(*this) == rhs &&
		m_Samplers == rhs.m_Samplers;
}
