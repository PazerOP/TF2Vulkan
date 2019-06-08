#pragma once

#include <TF2Vulkan/IBufferPool.h>
#include <TF2Vulkan/IShaderGroup.h>
#include <TF2Vulkan/IShaderInstance.h>
#include "TF2Vulkan/FogParams.h"
#include <TF2Vulkan/Util/Macros.h>

#include <stdshader_vulkan/ShaderDataShared.h>

#include <shaderapi/ishaderdynamic.h>

namespace TF2Vulkan
{
	union SamplerSettings;

	namespace Shaders
	{
		struct VSModelMatrices;
	}

	static constexpr size_t MAX_VS_LIGHTS = 4;

	class IShaderDynamicNext
	{
	protected:
		~IShaderDynamicNext() = default;

	public:
		virtual void SetShaderInstance(ShaderType type, const IShaderInstance* instance) = 0;
		virtual void BindUniformBuffer(const BufferPoolEntry& buf, UniformBufferIndex index) = 0;

		[[nodiscard]] virtual size_t GetLights(LightDesc_t* lights, size_t maxLights) const = 0;
		virtual void GetAmbientLightCube(Shaders::AmbientLightCube& cube) const = 0;

		virtual const LogicalFogParams& GetFogParams() const = 0;

		// From IShaderDynamicAPI
		virtual void BindStandardTexture(Sampler_t sampler, StandardTextureId_t id) = 0;
		virtual void GetWorldSpaceCameraPosition(Vector& pPos) const = 0;
		virtual void GetMatrix(MaterialMatrixMode_t matrixMode, VMatrix& dst) const = 0;

		virtual int GetCurrentNumBones() const = 0;
		virtual void LoadBoneMatrices(Shaders::VSModelMatrices& bones) const = 0;

		virtual void BindSamplers(const SamplerSettings* begin, const SamplerSettings* end, bool merge, uint32_t first) = 0;
		virtual void BindTextures(const ITexture* const* begin, const ITexture* const* end, bool merge, uint32_t first) = 0;

		// Helpers
		void SetPixelShader(const IShaderInstance* instance) { SetShaderInstance(ShaderType::Pixel, instance); }
		void SetPixelShader(const IShaderInstance& instance) { SetShaderInstance(ShaderType::Pixel, &instance); }
		void SetVertexShader(const IShaderInstance* instance) { SetShaderInstance(ShaderType::Vertex, instance); }
		void SetVertexShader(const IShaderInstance& instance) { SetShaderInstance(ShaderType::Vertex, &instance); }

		void BindUniformBuffer(const BufferPoolEntry* buf, UniformBufferIndex index)
		{
			if (buf)
				BindUniformBuffer(*buf, index);
		}

		template<size_t size> [[nodiscard]] size_t GetLights(LightDesc_t(&lights)[size]) const
		{
			static_assert(size == MAX_VS_LIGHTS);
			return GetLights(lights, size);
		}

		void BindSamplers(const SamplerSettings* begin, const SamplerSettings* end)
		{
			return BindSamplers(begin, end, false, 0);
		}
		void BindTextures(const ITexture* const* begin, const ITexture* const* end)
		{
			return BindTextures(begin, end, false, 0);
		}

	protected:
	};

#define SHADERDYNAMICNEXT_INTERFACE_VERSION "TF2Vulkan_IShaderDynamicNext001"
	extern IShaderDynamicNext* g_ShaderDynamic;
}
