#pragma once

#include "std_algorithm.h"
#include "std_utility.h"

#include <compare>
#include <stdexcept>
#include <type_traits>

namespace Util
{
	template<typename T, size_t maxSize>
	class InPlaceVector final
	{
	public:
		using value_type = T;
		using size_type = size_t;
		using reference = std::add_lvalue_reference_t<T>;
		using const_reference = std::add_lvalue_reference_t<std::add_const_t<T>>;
		using iterator = std::add_pointer_t<T>;
		using const_iterator = std::add_pointer_t<std::add_const_t<T>>;

		constexpr InPlaceVector() = default;
		InPlaceVector(const std::initializer_list<T>& values)
		{
			for (auto& val : values)
				push_back(val);
		}
		InPlaceVector(const InPlaceVector<T, maxSize>& other)
		{
			for (const auto& val : other)
				push_back(val);
		}
		InPlaceVector(InPlaceVector<T, maxSize>&& other)
		{
			for (auto& val : other)
				push_back(std::move(val));

			other.clear();
		}
		InPlaceVector& operator=(const InPlaceVector<T, maxSize>& other)
		{
			clear();

			for (const auto& val : other)
				push_back(val);

			return *this;
		}
		InPlaceVector& operator=(InPlaceVector<T, maxSize>&& other)
		{
			clear();

			for (auto& val : other)
				push_back(std::move(val));

			return *this;
		}
		~InPlaceVector()
		{
			clear();
		}

		template<size_type otherMaxSize>
		__declspec(noinline) bool operator==(const InPlaceVector<T, otherMaxSize>& other) const
		{
			if (size() != other.size())
				return false;

			return std::equal(begin(), end(), other.begin());
		}
		template<size_type otherMaxSize>
		__declspec(noinline)bool operator!=(const InPlaceVector<T, otherMaxSize>& other) const
		{
			return !operator==(other);
		}

		template<size_type otherMaxSize>
		bool operator<(const InPlaceVector<T, otherMaxSize>& other) const
		{
			return std::lexicographical_compare(
				begin(), end(),
				other.begin(), other.end());
		}
#ifdef __INTELLISENSE__

#else
		template<size_type otherMaxSize>
		auto operator<=>(const InPlaceVector<T, otherMaxSize>& other) const ->
			decltype(std::declval<T>() <=> other.data()[0])
		{
			if (auto result = size() <=> other.size(); !std::is_eq(result))
				return result;

			for (size_t i = 0; i < size(); i++)
			{
				auto result = operator[](i) <=> other[i];
				if (!std::is_eq(result))
					return result;
			}

			return std::strong_ordering::equal;
		}
		template<size_type otherMaxSize>
		auto operator<=>(const InPlaceVector<T, otherMaxSize>& other) const ->
			decltype(std::declval<T>() == other.data()[0], std::strong_equality::equal)
		{
			return operator==(other) ? std::strong_equality::equal : std::strong_equality::nonequal;
		}
#endif
		T& at(size_type i)
		{
			return const_cast<T&>(std::as_const(*this).at(i));
		}
		const T& at(size_type i) const
		{
			if (i >= size())
				throw std::out_of_range("Out of range index");

			return operator[](i);
		}

		T& operator[](size_type i) { return data()[i]; }
		const T& operator[](size_type i) const { return data()[i]; }

		T* data() { return reinterpret_cast<T*>(&m_Storage); }
		const T* data() const { return reinterpret_cast<const T*>(&m_Storage); }

		T* begin() { return data(); }
		const T* begin() const { return data(); }
		T* end() { return data() + size(); }
		const T* end() const { return data() + size(); }

		size_type size() const { return m_Elements; }
		static constexpr size_type max_size() { return maxSize; }

		T& front() { return at(0); }
		const T& front() const { return at(0); }
		T& back() { return at(size() - 1); }
		const T& back() const { return at(size() - 1); }

		void push_back(const T& val)
		{
			check_overflow(1);
			data()[m_Elements++] = val;
		}
		void push_back(T&& val)
		{
			check_overflow(1);
			data()[m_Elements++] = std::move(val);
		}

		void erase(const_iterator pos)
		{
			assert(size() > 0);
			assert(pos >= begin());
			assert(pos < end());

			std::move<iterator, iterator>(const_cast<iterator>(pos + 1), end(), const_cast<iterator>(pos));
			m_Elements--;
		}

		void pop_back()
		{
			erase(end() - 1);
		}

		template<typename... TArgs>
		T& emplace_back(TArgs&&... args)
		{
			push_back(T{ std::forward<TArgs>(args)... });
			return back();
		}

		void clear()
		{
			for (size_t i = 0; i < m_Elements; i++)
				data()[i].~T();

			m_Elements = 0;
		}

	private:
		void check_overflow(size_t addCount) const
		{
			if ((addCount + m_Elements) > maxSize)
				throw std::out_of_range("Exceeded capacity");
		}

		std::aligned_storage_t<sizeof(T[maxSize]), alignof(T[maxSize])> m_Storage;
		size_type m_Elements = 0;
	};
}

template<typename T, size_t maxSize> struct ::std::hash<::Util::InPlaceVector<T, maxSize>>
{
	size_t operator()(const ::Util::InPlaceVector<T, maxSize>& v) const
	{
		size_t hash = 0;
		for (const auto& val : v)
			hash = ::Util::hash_combine(hash, ::Util::hash_value(val));

		return hash;
	}
};
