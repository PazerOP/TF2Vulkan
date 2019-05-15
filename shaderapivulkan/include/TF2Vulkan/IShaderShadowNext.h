#pragma once

#include "IShaderInstance.h"
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

	private:
		void SetPixelShader(const char* name, int staticIndex) override final
		{
			NOT_IMPLEMENTED_FUNC();
		}
		void SetVertexShader(const char* name, int staticIndex) override final
		{
			NOT_IMPLEMENTED_FUNC();
		}
	};
}
