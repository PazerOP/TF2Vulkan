#pragma once

#include <malloc.h>

namespace Util{ namespace detail
{
	template<typename T>
	class StackArray
	{
	public:
		StackArray(void* ptr, size_t elements) :
			m_Ptr(reinterpret_cast<T*>(ptr)),
			m_Elements(elements)
		{
			for (size_t i = 0; i < elements; i++)
				::new(&at(i)) T;
		}
		StackArray(const StackArray&) = delete;
		StackArray(StackArray&&) = delete;
		StackArray& operator=(const StackArray&) = delete;
		StackArray& operator=(StackArray&&) = delete;
		~StackArray()
		{
			for (size_t i = 0; i < m_Elements; i++)
				m_Elements[i]->~T();

			_freea(m_Ptr);
		}

		constexpr size_t size() const { return m_Elements; }
		auto data() { return m_Ptr; }
		auto data() const { return m_Ptr; }

		auto begin() { return data(); }
		auto begin() const { return data(); }
		auto end() { return data() + size(); }
		auto end() const { return data() + size(); }

		auto& at(size_t i) { return const_cast<T&>(const_cast<const StackArray<T>*>(this)->at(i)); }
		auto& at(size_t i) const
		{
			if (i >= m_Elements)
				throw std::out_of_range();

			return m_Ptr[i];
		}

#ifdef VULKAN_HPP
		operator vk::ArrayProxy<T>() { return vk::ArrayProxy<T>(m_Elements, m_Ptr); }
		operator vk::ArrayProxy<const T>() const { return vk::ArrayProxy<const T>(m_Elements, m_Ptr); }
#endif

	private:
		T* m_Ptr;
		size_t m_Elements;
	};
} }

#define TF2VULKAN_STACKARRAY_NAMED(name, type, elements) \
	::Util::detail::StackArray<type> name(_malloca(sizeof(type) * elements), elements)

#define TF2VULKAN_STACKARRAY(type, elements) TF2VULKAN_STACKARRAY_NAMED(, type, elements)
