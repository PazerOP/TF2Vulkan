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

		template<typename T> struct ResourceRange
		{
			/// <summary>
			/// If this is nullptr, then this range of resources is left unmodified.
			/// </summary>
			const T* m_Resources = nullptr;
			size_t m_Count = 0;
			size_t m_Offset = 0;
		};
		using SamplerRange = ResourceRange<SamplerSettings>;
		using TextureRange = ResourceRange<const ITexture*>;

		/// <summary>
		/// Binds multiple contiguous ranges of SamplerSettings.
		/// </summary>
		/// <param name="merge">False to unbind any samplers not included in the ranges.</param>
		virtual void BindSamplerRanges(const SamplerRange* begin, const SamplerRange* end, bool merge) = 0;

		/// <summary>
		/// Binds multiple contiguous ranges of ITextures.
		/// </summary>
		/// <param name="merge">False to unbind any textures not included in the ranges.</param>
		virtual void BindTextureRanges(const TextureRange* begin, const TextureRange* end, bool merge) = 0;

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
			const SamplerRange range
			{
				.m_Resources = begin,
				.m_Count = size_t(end - begin),
			};
			return BindSamplerRanges(&range, &range + 1, false);
		}
		void BindTextures(const ITexture* const* begin, const ITexture* const* end)
		{
			const TextureRange range
			{
				.m_Resources = begin,
				.m_Count = size_t(end - begin),
			};
			return BindTextureRanges(&range, &range + 1, false);
		}
	};

#define SHADERDYNAMICNEXT_INTERFACE_VERSION "TF2Vulkan_IShaderDynamicNext001"
	extern IShaderDynamicNext* g_ShaderDynamic;
}
