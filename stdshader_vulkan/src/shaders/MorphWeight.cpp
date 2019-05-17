#include "BaseShaderNext.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

inline namespace MorphWeight
{
	class Shader final : public ShaderNext<Shader, EmptyParams, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "MorphWeight"; }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		void OnInitShader(IShaderNextFactory& factory) override
		{
			m_VSShader = &factory.FindOrCreateShaderGroup(ShaderType::Vertex, "xlitgeneric_vs");
			m_PSShader = &factory.FindOrCreateShaderGroup(ShaderType::Pixel, "xlitgeneric_ps");
		}

		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override {}

		void OnDrawElements(const OnDrawElementsParams& params) override
		{
			if (const auto& shadow = params.shadow)
			{
				shadow->EnableDepthTest(false);
				shadow->EnableDepthWrites(false);
				shadow->EnableAlphaWrites(true);
				shadow->EnableCulling(false);
				shadow->FogMode(SHADER_FOGMODE_DISABLED);

				shadow->SetShaders(m_VSShader, m_PSShader);

				int texCoord[2] = { 2, 4 };
				shadow->VertexShaderVertexFormat(VERTEX_FORMAT_USE_EXACT_FORMAT, std::size(texCoord), texCoord);
			}

			if (const auto& dynamic = params.dynamic)
			{
				dynamic->SetVertexShader(m_VSShader->FindOrCreateInstance());
				dynamic->SetPixelShader(m_PSShader->FindOrCreateInstance());
			}
		}

	private:
		IShaderGroup* m_VSShader = nullptr;
		IShaderGroup* m_PSShader = nullptr;
	};

	static const Shader::InstanceRegister s_MorphWeight;
}

DEFINE_NSHADER_FALLBACK(MorphWeight_DX9, MorphWeight);
