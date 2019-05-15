#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

#include <shaderlib/cshader.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct FillrateParams
	{
		NSHADER_PARAM(PASSCOUNT, SHADER_PARAM_TYPE_INTEGER, "1", "Number of passes for this material");
	};

	class Fillrate final : public ShaderNext<Fillrate, FillrateParams, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "Fillrate"; }

		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override {}
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:

	};

	static const Fillrate::InstanceRegister s_Fillrate;
}

void Fillrate::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}

void Fillrate::OnDrawElements(const OnDrawElementsParams& params)
{
	LOG_FUNC();

	auto& shadow = params.shadow;
	auto& dynamic = params.dynamic;

	if (shadow)
	{
		shadow->EnableDepthTest(false);
		shadow->EnableDepthWrites(false);
		shadow->EnableBlending(true);
		shadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE);

		// Set stream format (note that this shader supports compression)
		unsigned int flags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED;
		int nTexCoordCount = 1;
		int userDataSize = 0;
		shadow->VertexShaderVertexFormat(flags, nTexCoordCount, NULL, userDataSize);

		NOT_IMPLEMENTED_FUNC();
		//shadow->SetVertexShader("fillrate_vs");
		//shadow->SetPixelShader("fillrate_ps");
	}

	if (dynamic)
	{
		int numPasses = params[PASSCOUNT]->GetIntValue();
		float color[4];
		if (g_pConfig->bMeasureFillRate)
		{
			// have to multiply by 2/255 since pixel shader constant are 1.7.
			// Will divide the 2 out in the pixel shader.
			color[0] = numPasses * (2.0f / 255.0f);
		}
		else
		{
			color[0] = (16 * numPasses) * (2.0f / 255.0f);
		}
		color[1] = 0.0f;
		color[2] = 0.0f;
		color[3] = 0.0f;
		dynamic->SetPixelShaderConstant(0, color, 1);

		NOT_IMPLEMENTED_FUNC_NOBREAK();
		//DECLARE_DYNAMIC_VERTEX_SHADER(fillrate_vs20);
		//SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0);
		//SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
		//SET_DYNAMIC_VERTEX_SHADER(fillrate_vs20);
	}

	Draw();

	if (shadow)
	{
		shadow->EnableDepthTest(false);
		shadow->EnableDepthWrites(false);
		shadow->EnableBlending(true);
		shadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE);
		shadow->PolyMode(SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE);

		NOT_IMPLEMENTED_FUNC();
		//shadow->SetVertexShader("fillrate_vs");
		//shadow->SetPixelShader("fillrate_ps");
	}

	if (dynamic)
	{
		float color[4] = { 0, 0.05f, 0.05f, 0 };
		dynamic->SetPixelShaderConstant(0, color, 1);

		NOT_IMPLEMENTED_FUNC_NOBREAK();
		//DECLARE_DYNAMIC_VERTEX_SHADER(fillrate_vs20);
		//SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShaderAPI->GetCurrentNumBones() > 0);
		//SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
		//SET_DYNAMIC_VERTEX_SHADER(fillrate_vs20);
	}

	Draw();
}
