#include "ShaderParamNext.h"

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

ShaderParamNext::ShaderParamNext(const char* name, ShaderParamType_t type,
	const char* defaultVal, const char* help, int flags)
{
	m_Info.m_pName = name;
	m_Info.m_Type = type;
	m_Info.m_pDefaultValue = defaultVal;
	m_Info.m_pHelp = help;
	m_Info.m_nFlags = flags;
}

const char* ShaderParamNext::GetName() const
{
	return m_Info.m_pName;
}

const char* ShaderParamNext::GetHelp() const
{
	return m_Info.m_pHelp;
}

const char* ShaderParamNext::GetDefault() const
{
	return m_Info.m_pDefaultValue;
}

int ShaderParamNext::GetFlags() const
{
	return m_Info.m_nFlags;
}

ShaderParamType_t ShaderParamNext::GetType() const
{
	return m_Info.m_Type;
}

const ShaderParamInfo_t& ShaderParamNext::GetParamInfo() const
{
	return m_Info;
}

int ShaderParamNext::GetIndex() const
{
	assert(m_Index >= 0); // You must call InitShaderParamIndices on this block
	return m_Index;
}

ShaderParamNext::operator int() const
{
	return GetIndex();
}

void ShaderParamNext::InitIndex(int index)
{
	assert(m_Index == -1);
	m_Index = index;
}
