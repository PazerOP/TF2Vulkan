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

		bool operator!=(const SpecConstLayoutEntry& other) const noexcept { return !operator==(other); }
		bool operator==(const SpecConstLayoutEntry& other) const noexcept
		{
			return
				m_Name == other.m_Name &&
				m_Type == other.m_Type &&
				m_Offset == other.m_Offset;
		}
	};

	struct SpecConstLayoutCreateInfo final
	{
		const SpecConstLayoutEntry* m_Entries = nullptr;
		size_t m_EntryCount = 0;
	};

	class ISpecConstLayout
	{
	public:
		virtual const SpecConstLayoutEntry* GetEntries(size_t& count) const = 0;
		virtual size_t GetBufferSize() const = 0;
	};

#define SPEC_CONST_BUF_ENTRY(type, name) \
	::TF2Vulkan::SpecConstLayoutEntry name{ #name, GetSpecConstType<decltype(type::name)>(), offsetof(type, name) }
}

STD_HASH_DEFINITION(TF2Vulkan::SpecConstLayoutEntry,
	v.m_Name,
	v.m_Type,
	v.m_Offset
);
