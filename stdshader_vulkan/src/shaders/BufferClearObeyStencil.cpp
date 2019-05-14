#include "BaseShaderNext.h"
#include "ParamGroups.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct BufferClearObeyStencilParams
	{
		NSHADER_PARAM(CLEARCOLOR, SHADER_PARAM_TYPE_INTEGER, "1", "activates clearing of color");
		NSHADER_PARAM(CLEARALPHA, SHADER_PARAM_TYPE_INTEGER, "-1", "activates clearing of alpha. -1 == copy CLEARCOLOR setting");
		NSHADER_PARAM(CLEARDEPTH, SHADER_PARAM_TYPE_INTEGER, "1", "activates clearing of depth");
	};

	class BufferClearObeyStencil final : public ShaderNext<BufferClearObeyStencil, BufferClearObeyStencilParams, SHADER_NOT_EDITABLE>
	{
	public:
		const char* GetName() const override { LOG_FUNC(); return "BufferClearObeyStencil"; }

		// Inherited via ShaderNext
		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* params,
			const char* pMaterialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;
	};
}

static const BufferClearObeyStencil::InstanceRegister s_Shader;

void BufferClearObeyStencil::OnInitShaderInstance(IMaterialVar** params,
	IShaderInit* pShaderInit, const char* pMaterialName)
{
	LOG_FUNC();

	InitIntParam(CLEARALPHA, params, -1);
}

void BufferClearObeyStencil::OnDrawElements(const OnDrawElementsParams& params)
{
	LOG_FUNC();

	auto& shadow = params.shadow;
	auto& dynamic = params.dynamic;

	const bool bEnableColorWrites = params[CLEARCOLOR]->GetBoolValue();
	const bool bEnableAlphaWrites = (params[CLEARALPHA]->GetIntValue() >= 0) ? params[CLEARALPHA]->GetBoolValue() : bEnableColorWrites;

	if (shadow)
	{
		shadow->DepthFunc(SHADER_DEPTHFUNC_ALWAYS);
		shadow->EnableDepthWrites(params[CLEARDEPTH]->GetBoolValue());
		shadow->EnableColorWrites(bEnableColorWrites);
		shadow->EnableAlphaWrites(bEnableAlphaWrites);

		shadow->VertexShaderVertexFormat(VERTEX_POSITION | VERTEX_COLOR, 1);

		shadow->SetVertexShader("bufferclearobeystencil_vs");
		NOT_IMPLEMENTED_FUNC();
		/*shadow->SetPixelShader("bufferclearobeystencil_ps",
			{
				{ "USESCOLOR", bEnableColorWrites || bEnableAlphaWrites },
			});*/
	}

	if (dynamic)
	{
		// Nothing to do
	}

	Draw();
}
