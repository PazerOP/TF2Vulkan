#pragma once

#include <materialsystem/imaterialsystem.h>
#include <materialsystem/IShader.h>
#include <shaderlib/BaseShader.h>

namespace TF2Vulkan{ namespace Shaders
{
	class ShaderParamNext final
	{
	public:
		ShaderParamNext(const char* name, ShaderParamType_t type, const char* defaultVal,
			const char* help, int flags = 0);

		const char* GetName() const;
		const char* GetHelp() const;
		const char* GetDefault() const;
		int GetFlags() const;
		ShaderParamType_t GetType() const;
		const ShaderParamInfo_t& GetParamInfo() const;

		int GetIndex() const;
		operator int() const;

		void InitIndex(int index);

	private:
		ShaderParamInfo_t m_Info;
		int m_Index = -1;
	};
} }

#define NSHADER_PARAM(param, paramType, paramDefault, paramHelp) \
	::TF2Vulkan::Shaders::ShaderParamNext param = { "$" #param, paramType, paramDefault, paramHelp };
