#include "IVulkanShader.h"

#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/SafeConvert.h>

#include <stdexcept>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

IVulkanShader::IVulkanShader(const VulkanShaderParam* params, size_t paramCount) :
	m_Params(params), m_ParamCount(paramCount)
{
	assert(m_Params);
}

const char* IVulkanShader::GetParamName(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetName();

	return BaseClass::GetParamName(paramIndex);
}

const char* IVulkanShader::GetParamHelp(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetHelp();

	return BaseClass::GetParamHelp(paramIndex);
}

ShaderParamType_t IVulkanShader::GetParamType(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetType();

	return BaseClass::GetParamType(paramIndex);
}

const char* IVulkanShader::GetParamDefault(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetDefault();

	return BaseClass::GetParamDefault(paramIndex);
}

int IVulkanShader::GetParamFlags(int paramIndex) const
{
	LOG_FUNC();

	if (auto param = TryGetParam(paramIndex))
		return param->GetFlags();

	return BaseClass::GetParamFlags(paramIndex);
}

int IVulkanShader::GetNumParams() const
{
	return Util::SafeConvert<int>(m_ParamCount);
}

const VulkanShaderParam* IVulkanShader::TryGetParam(int paramIndex) const
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
