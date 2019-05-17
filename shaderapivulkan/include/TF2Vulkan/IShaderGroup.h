#pragma once

#include "ISpecConstLayout.h"

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
	};
	enum class TextureIndex : uint_fast16_t
	{
		Invalid = uint_fast16_t(-1),
	};

	enum class UniformBufferStandardType : uint_fast8_t
	{
		VSModelMatrices,
		ShaderCommon,
		ShaderCustom,
	};

	class IShaderInstance;
	class IShaderGroup
	{
	public:
		virtual const char* GetName() const = 0;
		virtual const ISpecConstLayout& GetSpecConstLayout() const = 0;
		virtual ShaderType GetShaderType() const = 0;

		virtual UniformBufferIndex FindUniformBuffer(const std::string_view& name) const = 0;
		UniformBufferIndex FindUniformBuffer(UniformBufferStandardType type) const;

		const IShaderInstance& FindOrCreateInstance()
		{
			return FindOrCreateInstance(nullptr, 0);
		}

		template<typename TBuffer>
		const IShaderInstance& FindOrCreateInstance(const BaseSpecConstBuffer<TBuffer>& buffer)
		{
			return FindOrCreateInstance(buffer.data(), buffer.size());
		}

	private:
		virtual IShaderInstance& FindOrCreateInstance(const void* specConstBuf, size_t specConstBufSize) = 0;
	};

	inline UniformBufferIndex IShaderGroup::FindUniformBuffer(UniformBufferStandardType type) const
	{
		switch (type)
		{
		case UniformBufferStandardType::VSModelMatrices: return FindUniformBuffer("VertexShaderModelMatrices");
		case UniformBufferStandardType::ShaderCommon:    return FindUniformBuffer("ShaderCommonConstants");
		case UniformBufferStandardType::ShaderCustom:    return FindUniformBuffer("ShaderCustomConstants");
		}

		assert(!"Unknown UniformBufferStandardType");
		return UniformBufferIndex::Invalid;
	}
}
