#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct DebugTangentSpaceParams
	{
		NSHADER_PARAM_OVERRIDE(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/basetexture", "unused", SHADER_PARAM_NOT_EDITABLE);
		NSHADER_PARAM_OVERRIDE(FRAME, SHADER_PARAM_TYPE_INTEGER, "0", "unused", SHADER_PARAM_NOT_EDITABLE);
		NSHADER_PARAM_OVERRIDE(BASETEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "unused", SHADER_PARAM_NOT_EDITABLE);
		NSHADER_PARAM_OVERRIDE(COLOR, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE);
		NSHADER_PARAM_OVERRIDE(ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE);
	};

	class DebugTangentSpace final : public ShaderNext<DebugTangentSpace, DebugTangentSpaceParams>
	{
	public:
		const char* GetName() const override { return "DebugTangentSpace"; }

		void OnInitShader(IShaderNextFactory& mgr) override { NOT_IMPLEMENTED_FUNC_NOBREAK(); }
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		// Inherited via ShaderNext
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		void OnDrawElements(const OnDrawElementsParams& params) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const DebugTangentSpace::InstanceRegister s_DebugTangentSpace;
}

void DebugTangentSpace::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}
