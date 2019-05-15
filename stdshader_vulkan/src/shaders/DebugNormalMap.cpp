#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct DebugNormalMapParams
	{
		NSHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map");
		NSHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap");
		NSHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform");
	};

	class DebugNormalMap final : public ShaderNext<DebugNormalMap, DebugNormalMapParams, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "DebugNormalMap"; }

		// Inherited via ShaderNext
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit,
			const char* pMaterialName) override {}
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:
		IShaderGroup* m_XLitGenericVS = nullptr;
		IShaderGroup* m_XLitGenericPS = nullptr;
	};

	static const DebugNormalMap::InstanceRegister s_DebugNormalMap;
}

void DebugNormalMap::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	LOG_FUNC();

	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}

void DebugNormalMap::OnDrawElements(const OnDrawElementsParams& params)
{
	LOG_FUNC();

	auto& shadow = params.shadow;
	auto& dynamic = params.dynamic;

	if (shadow)
	{
		shadow->EnableTexture(SHADER_SAMPLER0, true);

		// Set stream format (note that this shader supports compression)
		shadow->VertexShaderVertexFormat(VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED, 1);

		NOT_IMPLEMENTED_FUNC();
		//shadow->SetPixelShader(m_XLitGenericPS);
		//shadow->SetPixelShader(m_XLitGenericPS);
		/*shadow->SetVertexShader("xlitgeneric_vs",
			{
				{ "VERTEXCOLOR", false },
			});*/
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
}
