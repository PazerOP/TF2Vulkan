#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct DebugDepthParams
	{
	};

	class DebugDepth final : public ShaderNext<DebugDepth, DebugDepthParams>
	{
	public:
		const char* GetName() const override { return "DebugDepth"; }

		void OnInitShader(IShaderNextFactory& mgr) override { NOT_IMPLEMENTED_FUNC_NOBREAK(); }
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		virtual void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		virtual void OnDrawElements(const OnDrawElementsParams& params) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const DebugDepth::InstanceRegister s_DebugDepth;
}

void DebugDepth::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
}
