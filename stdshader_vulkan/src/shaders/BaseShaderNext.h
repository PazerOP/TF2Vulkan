#pragma once

#include "ShaderParamNext.h"
#include "ParamGroups.h"

#include <TF2Vulkan/IShaderDynamicNext.h>
#include <TF2Vulkan/IShaderShadowNext.h>

#include <shaderlib/BaseShader.h>
#include <shaderlib/cshader.h>
#include <shaderlib/ShaderDLL.h>

#include <optional>

namespace TF2Vulkan
{
	class IShaderNextFactory;
}

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

		void InitShader(IShaderNextFactory& factory);

	protected:
		void InitIntParam(int param, IMaterialVar** params, int defaultVal) const;
		void InitFloatParam(int param, IMaterialVar** params, float defaultVal) const;
		void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY) const;
		void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ) const;
		void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ, float defaultValW) const;
		virtual void OnInitShader(IShaderNextFactory& instanceMgr) = 0;

		struct OnDrawElementsParams
		{
			IMaterialVar** matvars;
			IShaderShadowNext* shadow;
			IShaderDynamicNext* dynamic;
			VertexCompressionType_t compression;
			CBasePerMaterialContextData** context;

			const IMaterialVar* operator[](const ShaderParamNext& var) const;
			const IMaterialVar* operator[](ShaderMaterialVars_t var) const;
		};

		virtual void OnDrawElements(const OnDrawElementsParams& params) = 0;

	private:
		void OnDrawElements(IMaterialVar** params, IShaderShadow* pShaderShadow,
			IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression,
			CBasePerMaterialContextData** pContextDataPtr) override final;

		const ShaderParamNext* TryGetParam(int paramIndex) const;

		[[nodiscard]] bool CheckParamIndex(int paramIndex) const;

		const ShaderParamNext* m_Params = nullptr;
		size_t m_ParamCount = 0;
		const ShaderParamNext* m_Overrides[NUM_SHADER_MATERIAL_VARS] = {};
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

	struct EmptyParams {};

	template<typename T, typename TParams = EmptyParams, ShaderFlags_t FLAGS = ShaderFlags_t(0)>
	class ShaderNext : public TParams, public BaseShaderNext
	{
	protected:
		using BaseClass = BaseShaderNext;
	public:
		ShaderNext() : BaseShaderNext(GetAsShaderParams(InitParamIndices(GetParamsObj())), GetShaderParamCount<TParams>())
		{
		}

		TParams& GetParamsObj() { return *this; }
		const TParams& GetParamsObj() const { return *this; }

		using InstanceRegister = DefaultInstanceRegister<T>;

		int GetFlags() const override final { return FLAGS; }
	};

#define DEFINE_NSHADER_FALLBACK(shaderName, fallbackShaderName) \
	class Fallback_ ## shaderName final : public ::TF2Vulkan::Shaders::ShaderNext<Fallback_ ## shaderName> \
	{ \
	public: \
		const char* GetName() const override { return #shaderName; } \
		const char* GetFallbackShader(IMaterialVar**) const override { return #fallbackShaderName; } \
		void OnInitShader(IShaderNextFactory&) override {} \
	protected: \
		void OnInitShaderInstance(IMaterialVar**, IShaderInit*, const char*) override { /*throw __FUNCSIG__;*/ } \
		void OnDrawElements(const OnDrawElementsParams& params) override { /*throw __FUNCSIG__*/ } \
	}; \
	static const Fallback_ ## shaderName ## ::InstanceRegister s_FallbackShader_ ## shaderName;
} }
