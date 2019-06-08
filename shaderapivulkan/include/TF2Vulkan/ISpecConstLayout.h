#pragma once

#include "AlignedTypes.h"

#include <TF2Vulkan/Util/std_string_view.h>
#include <TF2Vulkan/Util/std_utility.h>

namespace TF2Vulkan
{
	enum class SpecConstType
	{
		Bool,
		Int,
		UInt,
		Float,
	};

	template<typename T>
	static constexpr SpecConstType GetSpecConstType()
	{
		if constexpr (std::is_same_v<T, Shaders::bool32> || std::is_same_v<T, Shaders::bool1>)
			return SpecConstType::Bool;
		else if constexpr (std::is_same_v<T, Shaders::int1>)
			return SpecConstType::Int;
		else if constexpr (std::is_same_v<T, Shaders::uint1>)
			return SpecConstType::UInt;
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

		DEFAULT_STRONG_ORDERING_OPERATOR(SpecConstLayoutEntry);
	};

	struct SpecConstLayoutCreateInfo final
	{
		const SpecConstLayoutEntry* m_Entries = nullptr;
		size_t m_EntryCount = 0;

#ifndef __INTELLISENSE__
		std::strong_ordering operator<=>(const SpecConstLayoutCreateInfo& other) const noexcept
		{
			if (auto result = m_EntryCount <=> other.m_EntryCount; !std::is_eq(result))
				return result;

			for (size_t i = 0; i < m_EntryCount; i++)
			{
				if (auto result = m_Entries[i] <=> other.m_Entries[i]; !std::is_eq(result))
					return result;
			}

			return std::strong_ordering::equal;
		}
#endif

		auto begin() const { return m_Entries; }
		auto end() const { return m_Entries + m_EntryCount; }
	};

	class ISpecConstLayout
	{
	public:
		virtual const SpecConstLayoutCreateInfo& GetCreateInfo() const = 0;
		virtual size_t GetBufferSize() const = 0;

		const SpecConstLayoutEntry* GetEntries(size_t& count) const
		{
			auto& ci = GetCreateInfo();
			count = ci.m_EntryCount;
			return ci.m_Entries;
		}
	};

#define SPEC_CONST_BUF_ENTRY(type, name) \
	::TF2Vulkan::SpecConstLayoutEntry name{ #name, GetSpecConstType<decltype(type::name)>(), offsetof(type, name) }

	struct BaseSpecConstBuf
	{
		Shaders::uint1 TEXTURE2D_COUNT = 0;
		Shaders::uint1 SAMPLER_COUNT = 0;
	};

	template<typename T> struct BaseSpecConstLayout
	{
		SPEC_CONST_BUF_ENTRY(T, TEXTURE2D_COUNT);
		SPEC_CONST_BUF_ENTRY(T, SAMPLER_COUNT);
	};
}

STD_HASH_DEFINITION(TF2Vulkan::SpecConstLayoutEntry,
	v.m_Name,
	v.m_Type,
	v.m_Offset
);
