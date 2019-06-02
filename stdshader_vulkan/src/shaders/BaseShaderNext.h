#pragma once

#include "ShaderParamNext.h"
#include "ParamGroups.h"

#include <stdshader_vulkan/ShaderData.h>
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
		[[deprecated]] void InitIntParam(int param, IMaterialVar** params, int defaultVal) const;
		[[deprecated]] void InitFloatParam(int param, IMaterialVar** params, float defaultVal) const;
		[[deprecated]] void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY) const;
		[[deprecated]] void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ) const;
		[[deprecated]] void InitVecParam(int param, IMaterialVar** params, float defaultValX, float defaultValY, float defaultValZ, float defaultValW) const;
		virtual void OnInitShader(IShaderNextFactory& instanceMgr) = 0;

		void LoadLights(TF2Vulkan::Shaders::ShaderDataCommon& data) const;

		struct OnDrawElementsParams
		{
			IMaterialVar** matvars;
			IShaderShadowNext* shadow;
			IShaderDynamicNext* dynamic;
			VertexCompressionType_t compression;
			CBasePerMaterialContextData** context;

			operator IMaterialVar** () const { return matvars; }
			IMaterialVar* operator[](const ShaderParamNext& var);
			IMaterialVar* operator[](ShaderMaterialVars_t var);
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

		template<typename... TArgs>
		DefaultInstanceRegister(TArgs&&... args) :
			m_Instance(std::move(args)...)
		{
			GetShaderDLL()->InsertShader(&m_Instance);
		}

		bool IsRegistered() { return m_Instance.GetNumParams(); }
	};

	template<typename T, typename TComponents = ShaderComponents<>, ShaderFlags_t FLAGS = ShaderFlags_t(0)>
	class ShaderNext : public TComponents, public BaseShaderNext
	{
	protected:
		static_assert(is_shader_components_v<TComponents>);

		using BaseClass = BaseShaderNext;
		using Components = TComponents;

	public:
		ShaderNext() : BaseShaderNext(TComponents::ParamsBase(), TComponents::PARAMS_COUNT)
		{
		}

		void InitShaderParams(IMaterialVar** ppParams, const char* pMaterialName) override
		{
			BaseClass::InitShaderParams(ppParams, pMaterialName);
			TComponents::InitParamGroups(ppParams);
		}

		using InstanceRegister = DefaultInstanceRegister<T>;

		int GetFlags() const override final { return FLAGS; }
	};

#define DEFINE_NSHADER_FALLBACK(shaderName, fallbackShaderName) \
	class Fallback_ ## shaderName final : public ::TF2Vulkan::Shaders::ShaderNext<Fallback_ ## shaderName> \
	{ \
	public: \
		const char* GetName() const override { LOG_FUNC(); return #shaderName; } \
		const char* GetFallbackShader(IMaterialVar**) const override { LOG_FUNC(); return #fallbackShaderName; } \
		void OnInitShader(IShaderNextFactory&) override { LOG_FUNC(); } \
	protected: \
		void OnInitShaderInstance(IMaterialVar**, IShaderInit*, const char*) override { LOG_FUNC(); /*throw __FUNCSIG__;*/ } \
		void OnDrawElements(const OnDrawElementsParams& params) override { LOG_FUNC(); /*throw __FUNCSIG__*/ } \
	}; \
	static const Fallback_ ## shaderName ## ::InstanceRegister s_FallbackShader_ ## shaderName;
} }
