#pragma once

#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/std_utility.h>

#include <Color.h>

#include <vulkan/vulkan.hpp>

#include <variant>

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

namespace TF2Vulkan
{
	void TransitionImageLayout(const vk::Image& image, const vk::Format& format,
		const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
		const vk::CommandBuffer& cmdBuf, uint32_t mipLevel);

	template<typename T, size_t size>
	inline vk::ArrayProxy<T> to_array_proxy(T(&array)[size])
	{
		return vk::ArrayProxy<T>(size, array);
	}

	vk::Extent2D ToExtent2D(const vk::Extent3D& extent);

	static constexpr Color PIX_TF2VULKAN_RED(255, 93, 79);
	void SetPIXMarker(const vk::CommandBuffer& cmdBuf, const char* name, const Color& color = PIX_TF2VULKAN_RED);
	void SetPIXMarker(const vk::Queue& queue, const char* name, const Color& color = PIX_TF2VULKAN_RED);
	void BeginPIXEvent(const vk::CommandBuffer& cmdBuf, const char* name, const Color& color = PIX_TF2VULKAN_RED);
	void BeginPIXEvent(const vk::Queue& queue, const char* name, const Color& color = PIX_TF2VULKAN_RED);
	void EndPIXEvent(const vk::CommandBuffer& cmdBuf);
	void EndPIXEvent(const vk::Queue& queue);

	class PixScope final
	{
	public:
		template<typename... Args>
		PixScope(const char* fmt, const Args& ... args) : PixScope(PIX_TF2VULKAN_RED, fmt, args...) {}

		template<typename... Args>
		PixScope(const vk::CommandBuffer& cmdBuf, const char* fmt, const Args& ... args) :
			PixScope(cmdBuf, PIX_TF2VULKAN_RED, fmt, args...)
		{
		}

		template<typename... Args>
		PixScope(const vk::Queue& queue, const char* fmt, const Args& ... args) :
			PixScope(queue, PIX_TF2VULKAN_RED, fmt, args...)
		{
		}

		template<typename... Args>
		PixScope(const vk::CommandBuffer& cmdBuf, const Color& color, const char* fmt, const Args& ... args) :
			m_Object(cmdBuf)
		{
			char buf[512];
			sprintf_s(buf, fmt, args...);
			BeginPIXEvent(cmdBuf, buf, color);
		}
		template<typename... Args>
		PixScope(const vk::Queue& queue, const Color& color, const char* fmt, const Args& ... args) :
			m_Object(queue)
		{
			char buf[512];
			sprintf_s(buf, fmt, args...);
			BeginPIXEvent(queue, buf, color);
		}
		template<typename... Args>
		PixScope(const Color& color, const char* fmt, const Args&... args)
		{
			char buf[512];
			sprintf_s(buf, fmt, args...);
			BeginPIXEventAnon(buf, color);
		}

		~PixScope();

	private:
		void BeginPIXEventAnon(const char* fmt, const Color& color);
		std::variant<vk::Queue, vk::CommandBuffer> m_Object;
	};
}

#ifndef __INTELLISENSE__
inline std::strong_ordering operator<=>(const vk::ImageView& lhs, const vk::ImageView& rhs)
{
	return (VkImageView)lhs <=> (VkImageView)rhs;
}
inline std::strong_ordering operator<=>(const vk::Extent2D& lhs, const vk::Extent2D& rhs)
{
	auto result = lhs.width <=> rhs.width;
	if (!std::is_eq(result))
		return result;

	return lhs.width <=> rhs.width;
}
#endif
