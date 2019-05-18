#pragma once

#include "interface/internal/IVulkanCommandBuffer.h"

#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/std_utility.h>

#include <Color.h>
#include <tier0/platform.h>

#include <vulkan/vulkan.hpp>

template<typename T1, typename T2> struct ::std::hash<vk::Flags<T1, T2>>
{
	size_t operator()(const vk::Flags<T1, T2>& f) const
	{
		return Util::hash_value((T2)f);
	}
};

#pragma push_macro("VK_TYPE_HASH")
#undef VK_TYPE_HASH
#define VK_TYPE_HASH(type) STD_HASH_DEFINITION(vk:: ## type, (Vk ## type)v)
VK_TYPE_HASH(Image);
VK_TYPE_HASH(ImageView);
#pragma pop_macro("VK_TYPE_HASH")

STD_HASH_DEFINITION(vk::Extent2D,
	v.width,
	v.height
);

STD_HASH_DEFINITION(vk::Buffer, (VkBuffer)v);

STD_HASH_DEFINITION(vk::ComponentMapping,
	v.r,
	v.g,
	v.b,
	v.a
);

STD_HASH_DEFINITION(vk::ImageSubresourceRange,
	v.aspectMask,
	v.baseMipLevel,
	v.levelCount,
	v.baseArrayLayer,
	v.layerCount
);

STD_HASH_DEFINITION(vk::ImageViewCreateInfo,
	v.flags,
	v.image,
	v.viewType,
	v.format,
	v.components,
	v.subresourceRange
);

#define TF2VULKAN_MAKE_NONUNIQUE_STACK_COPY(varName, array) \
	auto varName = reinterpret_cast<std::add_pointer_t<std::decay_t<decltype(array[0].get())>>>( \
		stackalloc(std::size((array)) * sizeof(std::decay_t<decltype(array[0].get())>))); \
	for (size_t i = 0; i < std::size((array)); i++) \
		varName[i] = array[i].get();

namespace TF2Vulkan
{
	void TransitionImageLayout(const vk::Image& image, const vk::Format& format,
		const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
		IVulkanCommandBuffer& cmdBuf, uint32_t mipLevel);

	template<typename T, size_t size>
	inline vk::ArrayProxy<T> to_array_proxy(T(&array)[size])
	{
		return vk::ArrayProxy<T>(size, array);
	}
	template<typename T>
	inline vk::ArrayProxy<T> to_array_proxy(T* data, size_t count)
	{
		return vk::ArrayProxy<T>(count, data);
	}

	vk::Extent2D ToExtent2D(const vk::Extent3D& extent);
	vk::Extent3D ToExtent3D(const vk::Extent2D& extent);

	enum class ClearValueType : uint_fast8_t
	{
		Float,
		Int,
		UInt,
		DepthStencil,
	};

	enum class ClearColorType : uint_fast8_t
	{
		Float = ClearValueType::Float,
		Int = ClearValueType::Int,
		UInt = ClearValueType::UInt,
	};

	bool IsEqual(const vk::ClearValue& lhs, const vk::ClearValue& rhs, ClearValueType colorType);
	bool IsEqual(const vk::ClearColorValue& lhs, const vk::ClearColorValue& rhs, ClearColorType type);
}

#ifndef __INTELLISENSE__

#pragma push_macro("VULKAN_3WAY_COMPARISON_OP")
#undef VULKAN_3WAY_COMPARISON_OP
#define VULKAN_3WAY_COMPARISON_OP(base) \
	inline std::strong_ordering operator<=>(const vk::base& lhs, const vk::base& rhs) \
	{ \
		return (Vk ## base)lhs <=> (Vk ## base)rhs; \
	}

VULKAN_3WAY_COMPARISON_OP(Buffer);
VULKAN_3WAY_COMPARISON_OP(ImageView);

inline std::strong_ordering operator<=>(const vk::Extent2D& lhs, const vk::Extent2D& rhs)
{
	auto result = lhs.width <=> rhs.width;
	if (!std::is_eq(result))
		return result;

	return lhs.width <=> rhs.width;
}
#endif
