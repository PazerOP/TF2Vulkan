#include "BaseShaderNext.h"

#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	struct CompositorParams
	{
	};

	class Compositor final : public ShaderNext<Compositor, CompositorParams>
	{
	public:
		const char* GetName() const override { return "Compositor"; }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;

		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		void OnDrawElements(const OnDrawElementsParams& allParams) override
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
	};

	static const Compositor::InstanceRegister s_Compositor;
}
