#pragma once

#include "IShaderGroup.h"
#include <shaderapi/ishadershadow.h>

#include <TF2Vulkan/Util/Macros.h>

namespace TF2Vulkan
{
	class IShaderShadowNext : public IShaderShadow
	{
	public:
		virtual void SetShaderGroup(ShaderType type, IShaderGroup* group) = 0;
		void SetShaderGroup(ShaderType type, IShaderGroup& group) { return SetShaderGroup(type, &group); }
		void SetShaderGroup(IShaderGroup& group) { return SetShaderGroup(group.GetShaderType(), group); }

		// Various aliases
		void SetPixelShader(IShaderGroup* group) { return SetShaderGroup(ShaderType::Pixel, group); }
		void SetPixelShader(IShaderGroup& group) { return SetShaderGroup(ShaderType::Pixel, group); }
		void SetVertexShader(IShaderGroup* group) { return SetShaderGroup(ShaderType::Vertex, group); }
		void SetVertexShader(IShaderGroup& group) { return SetShaderGroup(ShaderType::Vertex, group); }
		void SetShaders(IShaderGroup* vertex, IShaderGroup* pixel)
		{
			SetShaderGroup(ShaderType::Vertex, vertex);
			SetShaderGroup(ShaderType::Pixel, pixel);
		}
		void SetShaders(IShaderGroup* vertex, IShaderGroup& pixel) { SetShaders(vertex, &pixel); }
		void SetShaders(IShaderGroup& vertex, IShaderGroup* pixel) { SetShaders(&vertex, pixel); }
		void SetShaders(IShaderGroup& vertex, IShaderGroup& pixel) { SetShaders(&vertex, &pixel); }

	private:
		[[deprecated]] void SetPixelShader(const char* name, int staticIndex) override final
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}
		[[deprecated]] void SetVertexShader(const char* name, int staticIndex) override final
		{
			NOT_IMPLEMENTED_FUNC_NOBREAK();
		}

		[[deprecated]] void EnableTexture(Sampler_t sampler, bool bEnable) override final {}
	};
}
