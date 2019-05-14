#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct MYSHADERParams
	{
	};

	class MYSHADER final : public ShaderNext<MYSHADER, MYSHADERParams>
	{
	public:
		const char* GetName() const override { return "MYSHADER"; }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		void OnDrawElements(const OnDrawElementsParams& params) override
		{
			NSHADER_EXPAND_ONDRAWELEMENTS();
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const MYSHADER::InstanceRegister s_MYSHADER;
}
