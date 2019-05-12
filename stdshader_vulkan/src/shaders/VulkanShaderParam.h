#pragma once

#include <materialsystem/imaterialsystem.h>
#include <materialsystem/IShader.h>
#include <shaderlib/BaseShader.h>

namespace TF2Vulkan{ namespace Shaders
{
	class VulkanShaderParam final
	{
	public:
		VulkanShaderParam(const char* name, ShaderParamType_t type, const char* defaultVal,
			const char* help, int index, int flags = 0);

		const char* GetName() const;
		const char* GetHelp() const;
		const char* GetDefault() const;
		int GetFlags() const;
		ShaderParamType_t GetType() const;
		const ShaderParamInfo_t& GetParamInfo() const;

		int GetIndex() const;
		operator int() const;

	private:
		ShaderParamInfo_t m_Info;
		int m_Index;
	};
} }

#define VSHADER_PARAM(param, paramType, paramDefault, paramHelp) \
	::TF2Vulkan::Shaders::VulkanShaderParam param = { "$" #param, paramType, paramDefault, paramHelp, s_ParamCount++ };
