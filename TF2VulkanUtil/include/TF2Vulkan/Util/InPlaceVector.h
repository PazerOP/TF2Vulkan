#pragma once

#include "std_utility.h"

#include <algorithm>
#include <compare>
#include <stdexcept>
#include <type_traits>

namespace Util
{
	template<typename T, size_t maxSize>
	class InPlaceVector final
	{
	public:
		using size_type = size_t;

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
