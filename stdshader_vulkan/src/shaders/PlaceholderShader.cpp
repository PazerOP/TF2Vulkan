#include "BaseShaderNext.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

inline namespace PlaceholderShader
{
	class Shader : public ShaderNext<Shader>
	{
	public:
		Shader(const char* name) : m_Name(name) {}

		const char* GetName() const override { return m_Name; }

		//void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShader(IShaderNextFactory& factory) override;

		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:
		const char* m_Name;
	};

	static const Shader::InstanceRegister s_PlaceholderShaders[] =
	{
		"BloomAdd",
		"BlurFilterX",
		"BlurFilterY",
		"BufferClearObeyStencil",
		"Cable",
		"Compositor",
		"DebugDepth",
		"DebugLuxels",
		"DebugNormalMap",
		"DebugTangentSpace",
		"DecalModulate",
		"Downsample",
		"Engine_Post",
		"Eyeglint",
		"Fillrate",
		"FloatCombine",
		"FloatToScreen",
		"Luminance_Compare",
		"Modulate",
		"MorphWeight",
		"MotionBlur",
		"Occlusion",
		"Pyro_Vision",
		"Refract",
		"Screenspace_General",
		"ParticleSphere",
		"ShadowBuild",
		"Sprite",
		"Spritecard",
		"UnlitTwoTexture",
		"WriteZ",
	};
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
