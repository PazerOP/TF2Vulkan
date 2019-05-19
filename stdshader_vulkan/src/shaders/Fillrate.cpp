#include "BaseShaderNext.h"

#include <TF2Vulkan/IBufferPool.h>
#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

#include <shaderlib/cshader.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

inline namespace Fillrate
{
	struct Params
	{
		NSHADER_PARAM(PASSCOUNT, SHADER_PARAM_TYPE_INTEGER, "1", "Number of passes for this material");
	};

	struct SpecConstBuf final : BaseSpecConstBuffer<SpecConstBuf>
	{
		bool1 SKINNING;
		int1 COMPRESSED_VERTS;
	};

	struct SpecConstLayoutInfo final : BaseSpecConstLayout<SpecConstLayoutInfo, SpecConstBuf>
	{
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, SKINNING);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, COMPRESSED_VERTS);

	} static constexpr s_SpecConstLayout;

	struct PSDataCustom final
	{
		float4 m_Color;
	};

	class Shader final : public ShaderNext<Shader, Params, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "Fillrate"; }

		void OnInitShader(IShaderNextFactory& mgr) override;
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override {}
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:
		IBufferPool* m_PSDataCustom = nullptr;
		UniformBufferIndex m_PSDataCustomIndex = UniformBufferIndex::Invalid;
		IShaderGroup* m_FillrateVS = nullptr;
		IShaderGroup* m_FillratePS = nullptr;
	};

	static const Shader::InstanceRegister s_Fillrate;
}

void Shader::OnInitShader(IShaderNextFactory& mgr)
{
	m_FillrateVS = &mgr.FindOrCreateShaderGroup(ShaderType::Vertex, "fillrate_vs", s_SpecConstLayout);
	m_FillratePS = &mgr.FindOrCreateShaderGroup(ShaderType::Pixel, "fillrate_ps");

	m_PSDataCustom = &mgr.GetUniformBufferPool();
	m_PSDataCustomIndex = m_FillratePS->FindUniformBuffer(UniformBufferStandardType::ShaderCustom);
}

void Shader::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}

void Shader::OnDrawElements(const OnDrawElementsParams& params)
{
	LOG_FUNC();

	auto& shadow = params.shadow;
	auto& dynamic = params.dynamic;

	PSDataCustom psData;

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

		shadow->SetVertexShader(m_FillrateVS);
		shadow->SetPixelShader(m_FillratePS);
	}

	if (dynamic)
	{
		int numPasses = params[PASSCOUNT]->GetIntValue();
		if (g_pConfig->bMeasureFillRate)
		{
			// have to multiply by 2/255 since pixel shader constant are 1.7.
			// Will divide the 2 out in the pixel shader.
			psData.m_Color[0] = numPasses * (2.0f / 255.0f);
		}
		else
		{
			psData.m_Color[0] = (16 * numPasses) * (2.0f / 255.0f);
		}
		psData.m_Color[1] = 0.0f;
		psData.m_Color[2] = 0.0f;
		psData.m_Color[3] = 0.0f;
		dynamic->BindUniformBuffer(m_PSDataCustom->Create(psData), m_PSDataCustomIndex);

		SpecConstBuf sc;
		sc.SKINNING = dynamic->GetCurrentNumBones() > 0;
		sc.COMPRESSED_VERTS = (int)params.compression;
		dynamic->SetVertexShader(m_FillrateVS->FindOrCreateInstance(sc));
	}

	Draw();

	if (shadow)
	{
		shadow->EnableDepthTest(false);
		shadow->EnableDepthWrites(false);
		shadow->EnableBlending(true);
		shadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE);
		shadow->PolyMode(SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE);
	}

	if (dynamic)
	{
		psData.m_Color.Set(0, 0.05f, 0.05f, 0);
		dynamic->BindUniformBuffer(m_PSDataCustom->Create(psData), m_PSDataCustomIndex);
	}

	Draw();
}
