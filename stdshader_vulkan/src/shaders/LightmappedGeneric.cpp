#include "BaseShaderNext.h"
#include "ParamGroups.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;
//using namespace TF2Vulkan::Shaders::Components;

#define SHADER_NAME LightmappedGeneric

inline namespace SHADER_NAME
{
	struct CustomParamGroup
	{
	};

	using Components = ShaderComponents<>;// /*CustomParamGroup, */DetailParams, SelfillumParams, BumpmapParams, SeamlessScaleParams>;

	class Shader final : public ShaderNext<Shader>
	{
	public:
		const char* GetName() const override { return V_STRINGIFY(SHADER_NAME); }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShader(IShaderNextFactory& factory) override;

		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:
		IShaderGroup* m_VSShader = nullptr;
		IShaderGroup* m_PSShader = nullptr;
	};

	// NOTE: THIS SHADER IS DISABLED! See XLitGeneric.cpp for where it's aliased to XLitGeneric.
	//static const Shader::InstanceRegister s_Shader;
}

//DEFINE_NSHADER_FALLBACK(WorldTwoTextureBlend, Shader);
//DEFINE_NSHADER_FALLBACK(WorldVertexTransition, Shader);
//DEFINE_NSHADER_FALLBACK(LightmappedReflective, Shader);
//DEFINE_NSHADER_FALLBACK(LightmappedTwoTexture, Shader);
//DEFINE_NSHADER_FALLBACK(LightmappedGeneric_Decal, Shader);
//DEFINE_NSHADER_FALLBACK(DecalBaseTimesLightmapAlphaBlendSelfIllum, Shader);

void Shader::OnInitShader(IShaderNextFactory& factory)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void Shader::OnInitShaderInstance(IMaterialVar** params, IShaderInit* init, const char* materialName)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void Shader::OnDrawElements(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}
