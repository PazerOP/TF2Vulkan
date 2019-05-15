#pragma once

#include "IShaderInstance.h"
#include "ISpecConstLayout.h"
#include <shaderapi/ishaderapi.h>

namespace TF2Vulkan
{
	class IUniformBufferManager;
	class IShaderNextInstanceMgr
	{
	public:
		virtual IUniformBufferManager* FindOrCreateUBM(size_t size) = 0;

		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name);
		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const ISpecConstLayout& layout);

		template<typename TInfo, typename TBuffer> ISpecConstLayout& FindOrCreateSpecConstLayout(
			const BaseSpecConstLayoutInfo<TInfo, TBuffer>& info);

	protected:
		virtual const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutEntry* entries, size_t count) = 0;

		virtual IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name,
			const ISpecConstLayout* layout) = 0;
	};

#define SHADERNEXTINSTANCEMGR_INTERFACE_VERSION "TF2Vulkan_IShaderNextInstanceMgr001"
	extern IShaderNextInstanceMgr& g_ShaderInstMgr;

	inline IShaderGroup& IShaderNextInstanceMgr::FindOrCreateShaderGroup(
		ShaderType type, const char* name, const ISpecConstLayout& layout)
	{
		return FindOrCreateShaderGroup(type, name, &layout);
	}

	inline IShaderGroup& IShaderNextInstanceMgr::FindOrCreateShaderGroup(
		ShaderType type, const char* name)
	{
		return FindOrCreateShaderGroup(type, name, nullptr);
	}

	template<typename TInfo, typename TBuffer>
	ISpecConstLayout& IShaderNextInstanceMgr::FindOrCreateSpecConstLayout(
		const BaseSpecConstLayoutInfo<TInfo, TBuffer>& info)
	{
		return FindOrCreateSpecConstLayout(info.data(), info.size());
	}
}
