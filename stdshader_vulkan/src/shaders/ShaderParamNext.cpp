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

ShaderParamNext::ShaderParamNext(ShaderMaterialVars_t overrideVar, ShaderParamType_t type,
	const char* defaultVal, const char* help, int flags) :
	m_Index(overrideVar)
{
	m_Info.m_pName = "override";
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

bool ShaderParamNext::InitIndex(int index)
{
	if (m_Index == -1)
	{
		m_Index = index;
		return true;
	}

	assert(m_Index >= 0 && m_Index < NUM_SHADER_MATERIAL_VARS);
	return false;
}

static bool CheckParamIndex(int index)
{
	if (index < 0)
	{
		assert(!"Invalid param index");
		return false;
	}

	return true;
}

void ShaderParamNext::InitParamInt(IMaterialVar** params, int defaultVal) const
{
	if (CheckParamIndex(m_Index) && !params[m_Index]->IsDefined())
		params[m_Index]->SetIntValue(defaultVal);
}

void ShaderParamNext::InitParamFloat(IMaterialVar** params, float defaultVal) const
{
	if (CheckParamIndex(m_Index) && !params[m_Index]->IsDefined())
		params[m_Index]->SetFloatValue(defaultVal);
}

void ShaderParamNext::InitParamVec(IMaterialVar** params, float defaultValX, float defaultValY) const
{
	if (CheckParamIndex(m_Index) && !params[m_Index]->IsDefined())
		params[m_Index]->SetVecValue(defaultValX, defaultValY);
}

void ShaderParamNext::InitParamVec(IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ) const
{
	if (CheckParamIndex(m_Index) && !params[m_Index]->IsDefined())
		params[m_Index]->SetVecValue(defaultValX, defaultValY, defaultValZ);
}

void ShaderParamNext::InitParamVec(IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ, float defaultValW) const
{
	if (CheckParamIndex(m_Index) && !params[m_Index]->IsDefined())
		params[m_Index]->SetVecValue(defaultValX, defaultValY, defaultValZ, defaultValW);
}
