#pragma once

#include "IShaderInstance.h"
#include <shaderapi/ishadershadow.h>

namespace TF2Vulkan
{
	class IShaderShadowNext : public IShaderShadow, public IShaderInstanceManager
	{
	public:
		virtual void SetPixelShader(const IPSInstance* instance) = 0;
		virtual void SetVertexShader(const IVSInstance* instance) = 0;

		void SetPixelShader(const char* name, const PSInstanceSettings& settings = {})
		{
			return SetPixelShader(FindOrCreatePSInstance(name, settings));
		}
		void SetVertexShader(const char* name, const VSInstanceSettings& settings = {})
		{
			return SetVertexShader(FindOrCreateVSInstance(name, settings));
		}

	private:
		void SetPixelShader(const char* name, int staticIndex) override final
		{
			assert(!"SetPixelShader with a static index is deprecated in IShaderShadowNext.");
		}
		void SetVertexShader(const char* name, int staticIndex) override final
		{
			assert(!"SetVertexShader with a static index is deprecated in IShaderShadowNext.");
		}
	};
}
