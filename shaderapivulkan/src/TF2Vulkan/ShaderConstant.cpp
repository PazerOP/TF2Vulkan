#include "ShaderConstant.h"

using namespace TF2Vulkan;

void ShaderConstantValues::SetData(size_t startReg, const ShaderConstant* data, size_t numRegs)
{
	for (size_t i = 0; i < numRegs; i++)
		m_Constants.at(startReg + i) = data[i];
}
