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
		ShaderParamNext(ShaderMaterialVars_t overrideVar, ShaderParamType_t type, const char* defaultVal,
			const char* help, int flags = 0);

		const char* GetName() const;
		const char* GetHelp() const;
		const char* GetDefault() const;
		int GetFlags() const;
		ShaderParamType_t GetType() const;
		const ShaderParamInfo_t& GetParamInfo() const;

		int GetIndex() const;
		operator int() const;

		[[nodiscard]] bool InitIndex(int index);

		void InitParamInt(IMaterialVar** params, int defaultVal) const;
		void InitParamFloat(IMaterialVar** params, float defaultVal) const;
		void InitParamVec(IMaterialVar** params, float defaultValX, float defaultValY) const;
		void InitParamVec(IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ) const;
		void InitParamVec(IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ, float defaultValW) const;

	private:
		ShaderParamInfo_t m_Info;
		int m_Index = -1;
	};
} }

#define NSHADER_PARAM(param, paramType, paramDefault, paramHelp) \
	::TF2Vulkan::Shaders::ShaderParamNext param = { "$" #param, paramType, paramDefault, paramHelp }

#define NSHADER_PARAM_OVERRIDE(param, paramType, paramDefault, paramHelp, flags) \
	::TF2Vulkan::Shaders::ShaderParamNext param = { static_cast<ShaderMaterialVars_t>(::param), paramType, paramDefault, paramHelp, flags }
