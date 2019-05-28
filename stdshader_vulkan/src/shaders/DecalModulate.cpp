#include "BaseShaderNext.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

#define SHADER_NAME DecalModulate

inline namespace SHADER_NAME
{
	struct Params
	{
	};

	class Shader final : public ShaderNext<Shader, Params>
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

	static const Shader::InstanceRegister s_Shader;
}

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
