#pragma once

#include "ISpecConstLayout.h"

#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/std_string.h>
#include <TF2Vulkan/Util/std_utility.h>
#include <TF2Vulkan/Util/std_variant.h>
#include <TF2Vulkan/Util/std_vector.h>

namespace TF2Vulkan
{
	class ISpecConstLayout;

	enum class ShaderType : uint_fast8_t
	{
		Pixel,
		Vertex,
	};

	class IShaderInstance;
	class IShaderGroup
	{
	public:
		virtual const char* GetName() const = 0;
		virtual const ISpecConstLayout& GetSpecConstLayout() const = 0;
		virtual ShaderType GetShaderType() const = 0;

		template<typename TBuffer>
		const IShaderInstance& FindOrCreateInstance(const BaseSpecConstBuffer<TBuffer>& buffer)
		{
			return FindOrCreateInstance(buffer.data, buffer.size());
		}

		template<typename T>
		const IShaderInstance& FindOrCreateInstance(const T& specConstBuf) const
		{
			static_assert(std::has_unique_object_representations_v<T>);
			return FindOrCreateInstance(&specConstBuf, sizeof(specConstBuf));
		}

	private:
		virtual IShaderInstance& FindOrCreateInstance(const void* specConstBuf, size_t specConstBufSize) = 0;
	};

	class IShaderInstance
	{
	public:
		IShaderGroup& GetGroup() { return const_cast<IShaderGroup&>(std::as_const(*this).GetGroup()); }
		virtual const IShaderGroup& GetGroup() const = 0;
		virtual const void* GetSpecConstBuffer() const = 0;
	};

	class IShaderInstanceManager
	{
	public:
		const IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name, const ISpecConstLayout& layout);

		template<typename TInfo, typename TBuffer> const ISpecConstLayout& FindOrCreateSpecConstLayout(
			const BaseSpecConstLayoutInfo<TInfo, TBuffer>& info);

		IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name);

	protected:
		virtual const ISpecConstLayout& FindOrCreateSpecConstLayout(const SpecConstLayoutEntry* entries, size_t count) = 0;

		virtual IShaderGroup& FindOrCreateShaderGroup(ShaderType type, const char* name,
			const ISpecConstLayout* layout) = 0;
	};

	inline const IShaderGroup& IShaderInstanceManager::FindOrCreateShaderGroup(
		ShaderType type, const char* name, const ISpecConstLayout& layout)
	{
		return FindOrCreateShaderGroup(type, name, &layout);
	}

	inline IShaderGroup& IShaderInstanceManager::FindOrCreateShaderGroup(
		ShaderType type, const char* name)
	{
		return FindOrCreateShaderGroup(type, name, nullptr);
	}

	template<typename TInfo, typename TBuffer>
	const ISpecConstLayout& IShaderInstanceManager::FindOrCreateSpecConstLayout(
		const BaseSpecConstLayoutInfo<TInfo, TBuffer>& info)
	{
		return FindOrCreateSpecConstLayout(info.data(), info.size());
	}
}
