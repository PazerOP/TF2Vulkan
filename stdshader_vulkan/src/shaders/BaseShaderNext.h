#pragma once

#include "ShaderParamNext.h"
#include "ParamGroups.h"

#include <TF2Vulkan/IShaderShadowNext.h>

#include <shaderlib/BaseShader.h>
#include <shaderlib/ShaderDLL.h>

#include <optional>

namespace TF2Vulkan{ namespace Shaders
{
	class BaseShaderNext : public CBaseShader
	{
		using BaseClass = CBaseShader;
	public:
		BaseShaderNext(const ShaderParamNext* params, size_t paramCount);

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

		virtual void OnDrawElements(IMaterialVar** params, IShaderShadowNext* shadow, IShaderDynamicAPI* dynamic,
			VertexCompressionType_t compression, CBasePerMaterialContextData** context) = 0;

	private:
		void OnDrawElements(IMaterialVar** params, IShaderShadow* pShaderShadow,
			IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression,
			CBasePerMaterialContextData** pContextDataPtr) override final;

		const ShaderParamNext* TryGetParam(int paramIndex) const;

		[[nodiscard]] bool CheckParamIndex(int paramIndex) const;

		std::optional<const ShaderParamNext> m_Overrides[NUM_SHADER_MATERIAL_VARS];
		const ShaderParamNext* m_Params = nullptr;
		size_t m_ParamCount = 0;
	};

	template<typename T>
	struct DefaultInstanceRegister
	{
		T m_Instance;
		DefaultInstanceRegister()
		{
			GetShaderDLL()->InsertShader(&m_Instance);
		}

		bool IsRegistered() { return m_Instance.GetNumParams(); }
	};

	template<typename T, typename TParams, ShaderFlags_t FLAGS = ShaderFlags_t(0)>
	class ShaderNext : public TParams, public BaseShaderNext
	{
	protected:
		using BaseClass = BaseShaderNext;
	public:
		ShaderNext() : BaseShaderNext(GetAsShaderParams(InitParamIndices(GetParams())), GetShaderParamCount<TParams>())
		{
		}

		TParams& GetParams() { return *this; }
		const TParams& GetParams() const { return *this; }

		using InstanceRegister = DefaultInstanceRegister<T>;

		int GetFlags() const override final { return FLAGS; }
	};
} }
