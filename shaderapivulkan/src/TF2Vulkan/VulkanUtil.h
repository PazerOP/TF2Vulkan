#pragma once

#include <TF2Vulkan/Util/std_utility.h>

#include <vulkan/vulkan.hpp>

template<typename T1, typename T2> struct ::std::hash<vk::Flags<T1, T2>>
{
	size_t operator()(const vk::Flags<T1, T2>& f) const
	{
		return Util::hash_value((T2)f);
	}
};

STD_HASH_DEFINITION(vk::Image, (VkImage)v);

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

namespace Util
{
	template<typename T, size_t size>
	inline vk::ArrayProxy<T> to_array_proxy(T(&array)[size])
	{
		return vk::ArrayProxy<T>(size, array);
	}
}
