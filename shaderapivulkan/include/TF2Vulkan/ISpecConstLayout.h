#pragma once

#include "AlignedTypes.h"

#include <TF2Vulkan/Util/std_utility.h>

#include <string_view>

namespace TF2Vulkan
{
	enum class SpecConstType
	{
		Bool,
		Int,
		Float,
	};

	template<typename T>
	static constexpr SpecConstType GetSpecConstType()
	{
		if constexpr (std::is_same_v<T, Shaders::bool32> || std::is_same_v<T, Shaders::bool1>)
			return SpecConstType::Bool;
		else if constexpr (std::is_same_v<T, Shaders::int1>)
			return SpecConstType::Int;
		else if constexpr (std::is_same_v<T, Shaders::float1>)
			return SpecConstType::Float;
		else
			static_assert(false, "Invalid spec const type");
	}

	struct SpecConstLayoutEntry
	{
		std::string_view m_Name;
		SpecConstType m_Type;
		size_t m_Offset;

		bool operator==(const SpecConstLayoutEntry& other) const noexcept
		{
			return
				m_Name == other.m_Name &&
				m_Type == other.m_Type &&
				m_Offset == other.m_Offset;
		}
	};

	template<typename TBuffer>
	struct BaseSpecConstBuffer
	{
		constexpr BaseSpecConstBuffer()
		{
			static_assert(std::has_unique_object_representations_v<TBuffer>,
				"Buffer must have no padding, so it can be memcmp'd");
		}

		const void* data() const { return reinterpret_cast<const void*>(this); }
		static constexpr size_t size()
		{
			static_assert(sizeof(TBuffer) % 4 == 0);
			return sizeof(TBuffer);
		}
	};

	template<typename TInfo, typename TBuffer>
	struct BaseSpecConstLayout
	{
		constexpr BaseSpecConstLayout()
		{
			static_assert(std::is_base_of_v<BaseSpecConstBuffer<TBuffer>, TBuffer>,
				"TBuffer must inherit from BaseSpecConstBuffer<TBuffer>");

			static_assert((sizeof(TInfo) / sizeof(SpecConstLayoutEntry)) == sizeof(TBuffer) / 4,
				"Mismatching element count for info struct and buffer struct");
		}

		static constexpr size_t size()
		{
			static_assert(sizeof(TInfo) % sizeof(SpecConstLayoutEntry) == 0);
			return sizeof(TInfo) / sizeof(SpecConstLayoutEntry);
		}
		const SpecConstLayoutEntry* data() const
		{
			return reinterpret_cast<const SpecConstLayoutEntry*>(this);
		}
		const SpecConstLayoutEntry& operator[](size_t i) const
		{
			assert(i < size());
			return *(data() + i);
		}

		auto begin() const { return data(); }
		auto end() const { return data() + size(); }
	};

#define SPEC_CONST_BUF_ENTRY(type, name) \
	::TF2Vulkan::SpecConstLayoutEntry name{ #name, GetSpecConstType<decltype(type::name)>(), offsetof(type, name) }

	class ISpecConstLayout
	{
	public:
		virtual const SpecConstLayoutEntry* GetEntries(size_t& count) const = 0;
		virtual size_t GetBufferSize() const = 0;
	};
}

STD_HASH_DEFINITION(TF2Vulkan::SpecConstLayoutEntry,
	v.m_Name,
	v.m_Type,
	v.m_Offset
);
