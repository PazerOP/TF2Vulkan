#pragma once

namespace Util{ namespace detail
{
	template<typename T, size_t elements>
	class StackBuffer
	{
		StackBuffer(void* ptr)
		{

		}

		static constexpr size_t size() { return sizeof(T) * elements; }

	private:
		T* m_Ptr;
	};
} }

#define TF2VULKAN_STACK_BUFFER(type, elements)
