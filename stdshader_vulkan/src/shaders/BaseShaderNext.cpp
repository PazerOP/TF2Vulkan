#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/SafeConvert.h>

#include <stdexcept>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

BaseShaderNext::BaseShaderNext(const ShaderParamNext* params, size_t paramCount) :
	m_Params(params), m_ParamCount(paramCount)
{
	assert(m_Params);
}

const char* BaseShaderNext::GetParamName(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetName();

	return BaseClass::GetParamName(paramIndex);
}

const char* BaseShaderNext::GetParamHelp(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetHelp();

	return BaseClass::GetParamHelp(paramIndex);
}

ShaderParamType_t BaseShaderNext::GetParamType(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetType();

	return BaseClass::GetParamType(paramIndex);
}

const char* BaseShaderNext::GetParamDefault(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetDefault();

	return BaseClass::GetParamDefault(paramIndex);
}

int BaseShaderNext::GetParamFlags(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetFlags();

	return BaseClass::GetParamFlags(paramIndex);
}

int BaseShaderNext::GetNumParams() const
{
	return Util::SafeConvert<int>(m_ParamCount) + BaseClass::GetNumParams();
}

bool BaseShaderNext::CheckParamIndex(int paramIndex) const
{
	if (paramIndex < 0 || paramIndex >= GetNumParams())
	{
		assert(!"Param index out of range");
		return false;
	}

	return true;
}

void BaseShaderNext::InitIntParam(int param, IMaterialVar** params, int defaultVal) const
{
	if (CheckParamIndex(param) && !params[param]->IsDefined())
		params[param]->SetIntValue(defaultVal);
}

void BaseShaderNext::InitFloatParam(int param, IMaterialVar** params, float defaultVal) const
{
	if (CheckParamIndex(param) && !params[param]->IsDefined())
		params[param]->SetFloatValue(defaultVal);
}

void BaseShaderNext::InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY) const
{
	if (CheckParamIndex(param) && !params[param]->IsDefined())
		params[param]->SetVecValue(defaultValX, defaultValY);
}

void BaseShaderNext::InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ) const
{
	if (CheckParamIndex(param) && !params[param]->IsDefined())
		params[param]->SetVecValue(defaultValX, defaultValY, defaultValZ);
}

void BaseShaderNext::InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ, float defaultValW) const
{
	if (CheckParamIndex(param) && !params[param]->IsDefined())
		params[param]->SetVecValue(defaultValX, defaultValY, defaultValZ, defaultValW);
}

void BaseShaderNext::OnDrawElements(IMaterialVar** params, IShaderShadow* pShaderShadow,
	IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression,
	CBasePerMaterialContextData** pContextDataPtr)
{
	auto shadowNext = dynamic_cast<IShaderShadowNext*>(pShaderShadow);
	if (pShaderShadow && !shadowNext)
	{
		assert(!"Invalid shader shadow hierarchy");
		return;
	}

	return OnDrawElements(params, dynamic_cast<IShaderShadowNext*>(pShaderShadow), pShaderAPI,
		vertexCompression, pContextDataPtr);
}

const ShaderParamNext* BaseShaderNext::TryGetParam(int paramIndex) const
{
	if (paramIndex < 0)
		throw std::runtime_error(__FUNCTION__ "(): Invalid parameter index");

	const auto baseParamCount = BaseClass::GetNumParams();
	if (paramIndex < baseParamCount)
	{
		if (size_t(paramIndex) < std::size(m_Overrides) && m_Overrides[paramIndex].has_value())
			return &*m_Overrides[paramIndex];
	}

	paramIndex -= baseParamCount;
	assert(paramIndex < GetNumParams());
	return &m_Params[paramIndex];
}
