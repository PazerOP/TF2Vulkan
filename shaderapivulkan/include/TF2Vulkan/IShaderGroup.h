#pragma once

#include "ISpecConstLayout.h"
#include "stdshader_vulkan/ShaderDataShared.h"

namespace TF2Vulkan
{
	enum class ShaderType : uint_fast8_t
	{
		Pixel,
		Vertex,
	};

	enum class UniformBufferIndex : uint_fast8_t
	{
		Invalid = uint_fast8_t(-1),

		ShaderCommon = TF2Vulkan::Shaders::BINDING_CBUF_SHADERCOMMON.x,
		ShaderCustom = TF2Vulkan::Shaders::BINDING_CBUF_SHADERCUSTOM.x,
		VSModelMatrices = TF2Vulkan::Shaders::BINDING_CBUF_VSMODELMATRICES.x,
	};
	enum class TextureIndex : uint_fast16_t
	{
		Invalid = uint_fast16_t(-1),
	};

	class IShaderInstance;
	class IShaderGroup
	{
	public:
		virtual const char* GetName() const = 0;
		virtual const ISpecConstLayout& GetSpecConstLayout() const = 0;
		virtual ShaderType GetShaderType() const = 0;

		virtual UniformBufferIndex FindUniformBuffer(const std::string_view& name) const = 0;

		virtual IShaderInstance& FindOrCreateInstance(const void* specConstBuf, size_t specConstBufSize) = 0;
		const IShaderInstance& FindOrCreateInstance() { return FindOrCreateInstance(nullptr, 0); }
		template<typename T> const IShaderInstance& FindOrCreateInstance(const T& specConstBuf)
		{
			return FindOrCreateInstance(&specConstBuf, sizeof(specConstBuf));
		}
	};
}
