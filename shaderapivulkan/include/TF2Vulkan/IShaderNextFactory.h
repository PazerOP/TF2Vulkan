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
		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const SpecConstLayoutCreateInfo& ci);

		virtual const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutCreateInfo& ci) = 0;

	private:
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

	inline IShaderGroup& IShaderNextFactory::FindOrCreateShaderGroup(ShaderType type,
		const char* name, const SpecConstLayoutCreateInfo& ci)
	{
		return FindOrCreateShaderGroup(type, name, FindOrCreateSpecConstLayout(ci));
	}
}
