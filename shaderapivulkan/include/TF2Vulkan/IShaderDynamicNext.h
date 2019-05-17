#pragma once

#include <TF2Vulkan/IUniformBufferPool.h>
#include <TF2Vulkan/IShaderGroup.h>
#include <TF2Vulkan/IShaderInstance.h>
#include <TF2Vulkan/Util/Macros.h>
#include <shaderapi/ishaderdynamic.h>

namespace TF2Vulkan
{
	class IShaderDynamicNext
	{
	protected:
		~IShaderDynamicNext() = default;

	public:
		virtual void SetShaderInstance(ShaderType type, const IShaderInstance* instance) = 0;
		virtual void BindUniformBuffer(UniformBuffer& buf, UniformBufferIndex index) = 0;

		// From IShaderDynamicAPI
		virtual int GetCurrentNumBones(void) const = 0;
		virtual void BindStandardTexture(Sampler_t sampler, StandardTextureId_t id) = 0;
		virtual void GetWorldSpaceCameraPosition(Vector& pPos) const = 0;
		virtual void GetMatrix(MaterialMatrixMode_t matrixMode, VMatrix& dst) const = 0;

		// Helpers
		void SetPixelShader(const IShaderInstance* instance) { SetShaderInstance(ShaderType::Pixel, instance); }
		void SetPixelShader(const IShaderInstance& instance) { SetShaderInstance(ShaderType::Pixel, &instance); }
		void SetVertexShader(const IShaderInstance* instance) { SetShaderInstance(ShaderType::Vertex, instance); }
		void SetVertexShader(const IShaderInstance& instance) { SetShaderInstance(ShaderType::Vertex, &instance); }

		void BindUniformBuffer(UniformBuffer* buf, UniformBufferIndex index)
		{
			if (buf)
				BindUniformBuffer(*buf, index);
		}
	};

#define SHADERDYNAMICNEXT_INTERFACE_VERSION "TF2Vulkan_IShaderDynamicNext001"
	extern IShaderDynamicNext* g_ShaderDynamic;
}
