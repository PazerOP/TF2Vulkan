#include "BaseShaderNext.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

inline namespace DebugNormalMap
{
	struct Params
	{
		NSHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map");
		NSHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap");
		NSHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform");
	};

	struct SpecConstBuf final : BaseSpecConstBuffer<SpecConstBuf>
	{
		bool1 VERTEXCOLOR;
	};

	struct SpecConstLayout final : BaseSpecConstLayout<SpecConstLayout, SpecConstBuf>
	{
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, VERTEXCOLOR);

	} static constexpr s_SpecConstLayout;

	class Shader final : public ShaderNext<Shader, Params, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "DebugNormalMap"; }

		void OnInitShader(IShaderNextFactory& mgr) override;
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit,
			const char* pMaterialName) override {}
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:
		IShaderGroup* m_XLitGenericVS = nullptr;
		IShaderGroup* m_XLitGenericPS = nullptr;
	};

	static const Shader::InstanceRegister s_DebugNormalMap;
}

void Shader::OnInitShader(IShaderNextFactory& mgr)
{
	m_XLitGenericVS = &mgr.FindOrCreateShaderGroup(ShaderType::Vertex, "xlitgeneric_vs", s_SpecConstLayout);
	m_XLitGenericPS = &mgr.FindOrCreateShaderGroup(ShaderType::Pixel, "xlitgeneric_ps", s_SpecConstLayout);
}

void Shader::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();

#if false
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
#endif
}

void Shader::OnDrawElements(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();

#if false

	auto& shadow = params.shadow;
	auto& dynamic = params.dynamic;

	if (shadow)
	{
		// Set stream format (note that this shader supports compression)
		shadow->VertexShaderVertexFormat(VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED, 1);

		shadow->SetShaders(m_XLitGenericVS, m_XLitGenericPS);
	}

	if (dynamic)
	{
		if (params[BUMPMAP]->IsTexture())
			BindTexture(SHADER_SAMPLER0, BUMPMAP, BUMPFRAME);
		else
			dynamic->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_NORMALMAP_FLAT);

		// TODO
		NOT_IMPLEMENTED_FUNC_NOBREAK();
		//SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BUMPTRANSFORM);
	}

	Draw();
#endif
}
