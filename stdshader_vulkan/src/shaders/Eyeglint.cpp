#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct EyeglintParams
	{
	};

	class Eyeglint final : public ShaderNext<Eyeglint, EyeglintParams>
	{
	public:
		const char* GetName() const override { return "Eyeglint"; }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		virtual void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		virtual void OnDrawElements(const OnDrawElementsParams& params) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const Eyeglint::InstanceRegister s_Eyeglint;
}
