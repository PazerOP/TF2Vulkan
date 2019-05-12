#include "VulkanShaderParam.h"

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

VulkanShaderParam::VulkanShaderParam(const char* name, ShaderParamType_t type,
	const char* defaultVal, const char* help, int flags)
{
	m_Info.m_pName = name;
	m_Info.m_Type = type;
	m_Info.m_pDefaultValue = defaultVal;
	m_Info.m_pHelp = help;
	m_Info.m_nFlags = flags;
}

const char* VulkanShaderParam::GetName() const
{
	return m_Info.m_pName;
}

const char* VulkanShaderParam::GetHelp() const
{
	return m_Info.m_pHelp;
}

const char* VulkanShaderParam::GetDefault() const
{
	return m_Info.m_pDefaultValue;
}

int VulkanShaderParam::GetFlags() const
{
	return m_Info.m_nFlags;
}

ShaderParamType_t VulkanShaderParam::GetType() const
{
	return m_Info.m_Type;
}

const ShaderParamInfo_t& VulkanShaderParam::GetParamInfo() const
{
	return m_Info;
}

int VulkanShaderParam::GetIndex() const
{
	assert(m_Index >= 0); // You must call InitShaderParamIndices on this block
	return m_Index;
}

VulkanShaderParam::operator int() const
{
	return GetIndex();
}

void VulkanShaderParam::InitIndex(int index)
{
	assert(m_Index == -1);
	m_Index = index;
}
