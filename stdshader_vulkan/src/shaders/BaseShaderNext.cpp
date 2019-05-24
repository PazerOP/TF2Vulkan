#include "BaseShaderNext.h"
#include "ShaderParamNext.h"

#include <stdshader_vulkan/ShaderData.h>
#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/SafeConvert.h>

#include <stdexcept>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

BaseShaderNext::BaseShaderNext(const ShaderParamNext* params, size_t paramCount) :
	m_Params(params), m_ParamCount(paramCount)
{
	assert(m_Params);

	for (size_t i = 0; i < m_ParamCount; i++)
	{
		auto& param = m_Params[i];
		assert(param.GetIndex() >= 0);
		if (param.GetIndex() < NUM_SHADER_MATERIAL_VARS)
			m_Overrides[param.GetIndex()] = &param;
	}
}

const char* BaseShaderNext::GetParamName(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
	{
		auto name = param->GetName();
		assert(name);
		return name;
	}

	return BaseClass::GetParamName(paramIndex);
}

const char* BaseShaderNext::GetParamHelp(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->GetHelp();

	return BaseClass::GetParamHelp(paramIndex);
}

ShaderParamType_t BaseShaderNext::GetParamType(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->GetType();

	return BaseClass::GetParamType(paramIndex);
}

const char* BaseShaderNext::GetParamDefault(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->GetDefault();

	return BaseClass::GetParamDefault(paramIndex);
}

int BaseShaderNext::GetParamFlags(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->GetFlags();

	return BaseClass::GetParamFlags(paramIndex);
}

int BaseShaderNext::GetNumParams() const
{
	return Util::SafeConvert<int>(m_ParamCount) + BaseClass::GetNumParams();
}

void BaseShaderNext::InitShader(IShaderNextFactory& factory)
{
	OnInitShader(factory);
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

void BaseShaderNext::LoadLights(ShaderDataCommon& data) const
{
	assert(s_pShaderAPI);
	const auto dynamic = g_ShaderDynamic;
	LightDesc_t currentLights[MAX_VS_LIGHTS];

	Util::SafeConvert(dynamic->GetLights(currentLights), data.m_LightCount);

	data.m_LightEnabled = {};

	for (size_t i = 0; i < data.m_LightCount; i++)
	{
		const LightDesc_t& lightIn = currentLights[i];
		LightInfo& lightOut = data.m_LightInfo[i];

		data.m_LightEnabled[i] = true;

		lightOut.bIsDirectional = lightIn.m_Type == MATERIAL_LIGHT_DIRECTIONAL ? 1.0f : 0.0f;
		lightOut.bIsSpot = lightIn.m_Type == MATERIAL_LIGHT_SPOT ? 1.0f : 0.0f;

		lightOut.color.Set(lightIn.m_Color.x, lightIn.m_Color.y, lightIn.m_Color.z);
		lightOut.dir.Set(lightIn.m_Direction.x, lightIn.m_Direction.y, lightIn.m_Direction.z);
		lightOut.pos.Set(lightIn.m_Position.x, lightIn.m_Position.y, lightIn.m_Position.z);

		if (lightIn.m_Type == MATERIAL_LIGHT_SPOT)
		{
			lightOut.falloff = lightIn.m_Falloff;

			auto& d1 = lightOut.stopdot1;
			auto& d2 = lightOut.stopdot2;
			d1 = cos(lightIn.m_Theta * 0.5f);
			d2 = cos(lightIn.m_Phi * 0.5f);
			lightOut.OOdot = (d1 > d2) ? 1.0f / (d1 - d2) : 0.0f;
		}
		else
		{
			lightOut.falloff = 0;
			lightOut.stopdot1 = 1;
			lightOut.stopdot2 = 1;
			lightOut.OOdot = 1;
		}

		lightOut.atten.Set(lightIn.m_Attenuation0, lightIn.m_Attenuation1, lightIn.m_Attenuation2);
	}
}

void BaseShaderNext::OnDrawElements(IMaterialVar** params, IShaderShadow* pShaderShadow,
	IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression,
	CBasePerMaterialContextData** pContextDataPtr)
{
	LOG_FUNC();

	OnDrawElementsParams fnParams;
	fnParams.matvars = params;
	fnParams.shadow = assert_cast<IShaderShadowNext*>(pShaderShadow);
	fnParams.dynamic = pShaderAPI ? g_ShaderDynamic : nullptr;
	fnParams.compression = vertexCompression;
	fnParams.context = pContextDataPtr;

	return OnDrawElements(fnParams);
}

const ShaderParamNext* BaseShaderNext::TryGetParam(int paramIndex) const
{
	if (paramIndex < 0)
		throw std::runtime_error(__FUNCTION__ "(): Invalid parameter index");

	const auto baseParamCount = BaseClass::GetNumParams();
	if (paramIndex < baseParamCount)
	{
		if (size_t(paramIndex) < std::size(m_Overrides) && m_Overrides[paramIndex])
			return &*m_Overrides[paramIndex];
		else
			return nullptr;
	}

	paramIndex -= baseParamCount;
	assert(paramIndex >= 0 && paramIndex < GetNumParams());
	return &m_Params[paramIndex];
}

IMaterialVar* BaseShaderNext::OnDrawElementsParams::operator[](const ShaderParamNext& var)
{
	return const_cast<IMaterialVar*>(Util::as_const(this)->operator[](var));
}

IMaterialVar* BaseShaderNext::OnDrawElementsParams::operator[](ShaderMaterialVars_t var)
{
	return const_cast<IMaterialVar*>(Util::as_const(this)->operator[](var));
}

const IMaterialVar* BaseShaderNext::OnDrawElementsParams::operator[](const ShaderParamNext& var) const
{
	return matvars[var.GetIndex()];
}

const IMaterialVar* BaseShaderNext::OnDrawElementsParams::operator[](ShaderMaterialVars_t var) const
{
	return matvars[var];
}
