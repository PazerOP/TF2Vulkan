#pragma once

#include <TF2Vulkan/IShaderInstance.h>
#include <TF2Vulkan/Util/Macros.h>

#include <shaderapi/ishaderapi.h>

namespace TF2Vulkan
{
	class IShaderDynamicNext : public IShaderDynamicAPI
	{
	public:
		virtual void SetShaderInstance(ShaderType type, const IShaderInstance* instance) = 0;

		void SetPixelShader(const IShaderInstance* instance) { SetShaderInstance(ShaderType::Pixel, instance); }
		void SetPixelShader(const IShaderInstance& instance) { SetShaderInstance(ShaderType::Pixel, &instance); }
		void SetVertexShader(const IShaderInstance* instance) { SetShaderInstance(ShaderType::Vertex, instance); }
		void SetVertexShader(const IShaderInstance& instance) { SetShaderInstance(ShaderType::Vertex, &instance); }

		// Const-correctness fixes

		[[deprecated("Use SetPixelShader")]] void SetPixelShaderIndex(int vshIndex) override final { NOT_IMPLEMENTED_FUNC(); }
		[[deprecated("Use SetVertexShader")]] void SetVertexShaderIndex(int index) override final { NOT_IMPLEMENTED_FUNC(); }
	};

#define SHADERDYNAMICNEXT_INTERFACE_VERSION "TF2Vulkan_IShaderDynamicNext001"
	extern IShaderDynamicNext* g_ShaderDynamic;
}
