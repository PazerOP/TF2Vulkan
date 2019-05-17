#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct DebugDrawEnvmapMaskParams
	{
		NSHADER_PARAM(SHOWALPHA, SHADER_PARAM_TYPE_INTEGER, "0", "");
	};

	class DebugDrawEnvmapMask : public ShaderNext<DebugDrawEnvmapMask, DebugDrawEnvmapMaskParams, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { return "DebugDrawEnvmapMask"; }

		// Inherited via ShaderNext
		void OnInitShader(IShaderNextFactory& mgr) override { NOT_IMPLEMENTED_FUNC_NOBREAK(); }
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}

		void OnDrawElements(const OnDrawElementsParams& params) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const DebugDrawEnvmapMask::InstanceRegister s_DebugDrawEnvmapMask;
}
