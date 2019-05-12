#pragma once

#include "VulkanShaderParam.h"

#include <shaderlib/BaseShader.h>

#include <optional>

namespace TF2Vulkan{ namespace Shaders
{
	class IVulkanShader : public CBaseShader
	{
		using BaseClass = CBaseShader;
	public:
		IVulkanShader(const VulkanShaderParam* params, size_t paramCount);

		const char* GetParamName(int paramIndex) const override final;
		const char* GetParamHelp(int paramIndex) const override final;
		ShaderParamType_t GetParamType(int paramIndex) const override final;
		const char* GetParamDefault(int paramIndex) const override final;
		int GetParamFlags(int paramIndex) const override final;
		int GetNumParams() const override final;

	protected:
		void InitIntParam(int param, IMaterialVar** params, int defaultVal) const;
		void InitFloatParam(int param, IMaterialVar** params, float defaultVal) const;
		void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY) const;
		void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ) const;
		void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ, float defaultValW) const;

	private:
		const VulkanShaderParam* TryGetParam(int paramIndex) const;

		[[nodiscard]] bool CheckParamIndex(int paramIndex) const;

		std::optional<const VulkanShaderParam> m_Overrides[NUM_SHADER_MATERIAL_VARS];
		const VulkanShaderParam* m_Params = nullptr;
		size_t m_ParamCount = 0;
	};
} }
