#pragma once

#include "FormatConversion.h"

#include <materialsystem/imaterial.h>

#include <TF2Vulkan/Util/Enums.h>
#include <TF2Vulkan/Util/std_utility.h>

#include <compare>

namespace TF2Vulkan
{
	enum class VertexFormatFlags : VertexFormat_t
	{
		Position = VERTEX_POSITION,
		Normal = VERTEX_NORMAL,
		Color = VERTEX_COLOR,
		Specular = VERTEX_SPECULAR,

		TangentS = VERTEX_TANGENT_S,
		TangentT = VERTEX_TANGENT_T,
		//TangentSpace = TangentS | TangentT,

		Wrinkle = VERTEX_WRINKLE,
		BoneIndex = VERTEX_BONE_INDEX,

		Meta_VertexShader = VERTEX_FORMAT_VERTEX_SHADER,
		Meta_UseExactFormat = VERTEX_FORMAT_USE_EXACT_FORMAT,
		Meta_Compressed = VERTEX_FORMAT_COMPRESSED,
	};

	union VertexFormat
	{
		constexpr explicit VertexFormat(VertexFormat_t baseFmt = VERTEX_FORMAT_UNKNOWN) :
			m_BaseFmt(baseFmt)
		{
		}

#ifdef __INTELLISENSE__
		constexpr bool operator==(const VertexFormat& other) const { return m_BaseFmt == other.m_BaseFmt; }
		constexpr bool operator!=(const VertexFormat& other) const { return !operator==(other); }
#else
		constexpr std::strong_ordering operator<=>(const VertexFormat& other) const { return m_BaseFmt <=> other.m_BaseFmt; }
#endif

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		constexpr bool operator==(const T& other) const { return m_BaseFmt == other; }
		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		constexpr bool operator!=(const T& other) const { return !operator==(other); }

		constexpr operator VertexFormat_t() const { return m_BaseFmt; }

		size_t GetVertexSize() const;
		uint_fast8_t GetTexCoordSize(uint_fast8_t index) const;
		VertexCompressionType_t GetCompressionType() const;

		void SetCompressionEnabled(bool enabled);

		struct ElementType final
		{
			constexpr ElementType(VertexElement_t elem, DataFormat fmt, uint_fast8_t compCount,
				uint_fast8_t compSize, const std::string_view& semantic);

			VertexElement_t m_Element = VERTEX_ELEMENT_NONE;
			DataFormat m_Format = DataFormat::Invalid;
			uint_fast8_t m_Components = 0;
			uint_fast8_t m_ComponentSize = 0;
			std::string_view m_Semantic;

			uint_fast8_t GetTotalSize() const;
			vk::Format GetVKFormat() const;
		};

		struct Element
		{
			const ElementType* m_Type = nullptr;
			uint_fast8_t m_Offset = 0;
		};

		[[nodiscard]] uint_fast8_t GetVertexElements(Element* elements, uint_fast8_t maxElements, size_t* totalSize = nullptr) const;

		VertexFormat_t m_BaseFmt;
		struct
		{
			VertexFormatFlags m_Flags : VERTEX_LAST_BIT + 1;
			uint64_t m_BoneWeightCount : 3;
			uint64_t m_UserDataSize : 3;
			uint64_t m_TexCoordSize0 : 3;
			uint64_t m_TexCoordSize1 : 3;
			uint64_t m_TexCoordSize2 : 3;
			uint64_t m_TexCoordSize3 : 3;
			uint64_t m_TexCoordSize4 : 3;
			uint64_t m_TexCoordSize5 : 3;
			uint64_t m_TexCoordSize6 : 3;
			uint64_t m_TexCoordSize7 : 3;
		};
	};
	static_assert(sizeof(VertexFormat) == sizeof(VertexFormat_t));
}

ENABLE_ENUM_FLAG_OPS(TF2Vulkan::VertexFormatFlags);

namespace TF2Vulkan
{
	static constexpr VertexFormatFlags VFF_TangentSpace = VertexFormatFlags::TangentS | VertexFormatFlags::TangentT;
}

STD_HASH_DEFINITION(TF2Vulkan::VertexFormat,
	v.m_BaseFmt
);
