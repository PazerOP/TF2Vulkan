#pragma once

#include "IShaderInstance.h"
#include <shaderapi/ishadershadow.h>

namespace TF2Vulkan
{
	class IShaderShadowNext : public IShaderShadow, public IShaderInstanceManager
	{
	public:
		virtual void SetShaderGroup(ShaderType type, IShaderGroup* group) = 0;
		void SetShaderGroup(ShaderType type, IShaderGroup& group) { return SetShaderGroup(type, &group); }
		void SetShaderGroup(IShaderGroup& group) { return SetShaderGroup(group.GetShaderType(), group); }

		// Various aliases
		void SetShaderGroup(ShaderType type, const char* name) { return SetShaderGroup(type, FindOrCreateShaderGroup(type, name)); }
		void SetPixelShader(const char* name) { return SetShaderGroup(ShaderType::Pixel, name); }
		void SetVertexShader(const char* name) { return SetShaderGroup(ShaderType::Vertex, name); }

	private:
		void SetPixelShader(const char* name, int staticIndex) override final
		{
			assert(staticIndex == 0 && "SetPixelShader with a static index is deprecated in IShaderShadowNext.");
			return SetPixelShader(name);
		}
		void SetVertexShader(const char* name, int staticIndex) override final
		{
			assert(staticIndex == 0 && "SetVertexShader with a static index is deprecated in IShaderShadowNext.");
			return SetVertexShader(name);
		}
	};
}
