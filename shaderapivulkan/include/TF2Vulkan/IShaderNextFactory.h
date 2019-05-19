#pragma once

#include "IShaderInstance.h"
#include "ISpecConstLayout.h"
#include <shaderapi/ishaderapi.h>

namespace TF2Vulkan
{
	class IBufferPool;
	class IShaderNextFactory
	{
	public:
		virtual IBufferPool& GetUniformBufferPool() = 0;

		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name);
		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const ISpecConstLayout& layout);

		template<typename TInfo, typename TBuffer> IShaderGroup& FindOrCreateShaderGroup(
			ShaderType type, const char* name, const BaseSpecConstLayout<TInfo, TBuffer>& layout);

		template<typename TInfo, typename TBuffer> const ISpecConstLayout& FindOrCreateSpecConstLayout(
			const BaseSpecConstLayout<TInfo, TBuffer>& info);

	protected:
		virtual const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutEntry* entries, size_t count) = 0;

		virtual IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name,
			const ISpecConstLayout* layout) = 0;
	};

#define SHADERNEXTFACTORY_INTERFACE_VERSION "TF2Vulkan_IShaderNextFactory001"
	extern IShaderNextFactory* g_ShaderFactory;

	inline IShaderGroup& IShaderNextFactory::FindOrCreateShaderGroup(
		ShaderType type, const char* name, const ISpecConstLayout& layout)
	{
		return FindOrCreateShaderGroup(type, name, &layout);
	}

	inline IShaderGroup& IShaderNextFactory::FindOrCreateShaderGroup(
		ShaderType type, const char* name)
	{
		return FindOrCreateShaderGroup(type, name, nullptr);
	}

	template<typename TInfo, typename TBuffer>
	inline const ISpecConstLayout& IShaderNextFactory::FindOrCreateSpecConstLayout(
		const BaseSpecConstLayout<TInfo, TBuffer>& info)
	{
		return FindOrCreateSpecConstLayout(info.data(), info.size());
	}

	template<typename TInfo, typename TBuffer>
	inline IShaderGroup& IShaderNextFactory::FindOrCreateShaderGroup(ShaderType type,
		const char* name, const BaseSpecConstLayout<TInfo, TBuffer>& layout)
	{
		return FindOrCreateShaderGroup(type, name, FindOrCreateSpecConstLayout(layout));
	}
}
