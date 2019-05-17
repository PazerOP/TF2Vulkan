#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct DepthWriteParams
	{
		NSHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "", "Alpha reference value");
		NSHADER_PARAM(DISPLACEMENTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Displacement map");
		NSHADER_PARAM(DISPLACEMENTWRINKLE, SHADER_PARAM_TYPE_BOOL, "0", "Displacement map contains wrinkle displacements");
	};

	class DepthWrite final : public ShaderNext<DepthWrite, DepthWriteParams, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "DepthWrite"; }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		void OnInitShader(IShaderNextFactory& mgr) override { NOT_IMPLEMENTED_FUNC_NOBREAK(); }
		virtual void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		virtual void OnDrawElements(const OnDrawElementsParams& params) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const DepthWrite::InstanceRegister s_DepthWrite;
}
