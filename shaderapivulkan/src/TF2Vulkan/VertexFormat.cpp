#include "stdafx.h"
#include "VertexFormat.h"

#include <materialsystem/imesh.h>

using namespace TF2Vulkan;

static const std::array<VertexFormat::ElementType, VERTEX_ELEMENT_NUMELEMENTS> s_ElementTypes = []
{
	std::array<VertexFormat::ElementType, VERTEX_ELEMENT_NUMELEMENTS> retVal;

	for (int i = 0; i < VERTEX_ELEMENT_NUMELEMENTS; i++)
	{
		auto& e = retVal[i];
		e.m_Type = VertexElement_t(i);

		// Data format
		switch (i)
		{
		case VERTEX_ELEMENT_COLOR:
		case VERTEX_ELEMENT_SPECULAR:
			e.m_Format = DataFormat::UNorm;
			break;

		case VERTEX_ELEMENT_NORMAL:
			e.m_Format = DataFormat::UInt;
			break;

		case VERTEX_ELEMENT_BONEWEIGHTS1:
		case VERTEX_ELEMENT_BONEWEIGHTS2:
			e.m_Format = DataFormat::SInt;
			break;

		case VERTEX_ELEMENT_BONEWEIGHTS3:
		case VERTEX_ELEMENT_BONEWEIGHTS4:
			e.m_Format = DataFormat::SFloat;
			break;
		}
		assert(e.m_Format != DataFormat::Invalid);

		// Component count
		switch (i)
		{
		case VERTEX_ELEMENT_WRINKLE:
			e.m_Components = 1;
			break;

		case VERTEX_ELEMENT_POSITION:
		case VERTEX_ELEMENT_NORMAL:
		case VERTEX_ELEMENT_TANGENT_S:
		case VERTEX_ELEMENT_TANGENT_T:
			e.m_Components = 3;
			break;

		case VERTEX_ELEMENT_COLOR:
		case VERTEX_ELEMENT_SPECULAR:
			e.m_Components = 4;
			break;
		}
		assert(e.m_Components > 0);
	}

	return retVal;
}();

size_t VertexFormat::GetVertexSize() const
{
	using VFF = VertexFormatFlags;
	size_t size = 0;

	NOT_IMPLEMENTED_FUNC();

	return size;
}

uint_fast8_t VertexFormat::GetTexCoordSize(uint_fast8_t index) const
{
	return TexCoordSize(index, m_BaseFmt);
}

VertexCompressionType_t VertexFormat::GetCompressionType() const
{
	if (m_Flags & VertexFormatFlags::Meta_Compressed)
		return VERTEX_COMPRESSION_ON;

	return VERTEX_COMPRESSION_NONE;
}

void VertexFormat::SetCompressionEnabled(bool enabled)
{
	if (enabled)
		m_Flags = m_Flags | VertexFormatFlags::Meta_Compressed;
	else
		m_Flags = m_Flags & ~VertexFormatFlags::Meta_Compressed;
}

uint_fast8_t VertexFormat::GetVertexElements(Element* elements, uint_fast8_t maxElements, size_t* totalSize) const
{
	assert(maxElements >= VERTEX_ELEMENT_NUMELEMENTS);

	using VFF = VertexFormatFlags;
	size_t totalSizeTemp = 0;
	uint_fast8_t elemCount = 0;

	const auto compression = GetCompressionType();

	const auto StoreElement = [&](VertexElement_t elemType)
	{
		assert(elemType < VERTEX_ELEMENT_NUMELEMENTS);
		if (elemCount >= maxElements)
			return;

#ifdef _DEBUG
		if (elemCount > 0)
		{
			auto prev = elements[elemCount - 1].m_Type->m_Type;
			if (prev >= VERTEX_ELEMENT_TEXCOORD1D_0)
			{
				auto prevIdx = (prev - VERTEX_ELEMENT_TEXCOORD1D_0) % VERTEX_ELEMENT_NUMELEMENTS;
				auto thisIdx = (elemType - VERTEX_ELEMENT_TEXCOORD1D_0) % VERTEX_ELEMENT_NUMELEMENTS;
				assert(prevIdx < thisIdx);
			}
			else
			{
				assert(prev < elemType);
			}
		}
#endif

		Element& e = elements[elemCount++];
		e.m_Type = elemType;
		e.m_Size = GetVertexElementSize(elemType, compression);

		switch (e.m_Type)
		{
		case VERTEX_ELEMENT_NORMAL:
			e.m_Format = vk::Format::eR8G8B8A8Uint;
			break;
		case VERTEX_ELEMENT_COLOR:
			e.m_Format = vk::Format::eR8G8B8A8Unorm;
			break;

		default:
			e.m_Format =
		}

		e.m_Offset = totalSizeTemp;
		totalSizeTemp += e.m_Size;
	};

	if (m_Flags & VFF::Position)
		StoreElement(VERTEX_ELEMENT_POSITION);
	if (m_Flags & VFF::Normal)
		StoreElement(VERTEX_ELEMENT_NORMAL);
	if (m_Flags & VFF::Color)
		StoreElement(VERTEX_ELEMENT_COLOR);
	if (m_Flags & VFF::Specular)
		StoreElement(VERTEX_ELEMENT_SPECULAR);
	if (m_Flags & VFF::TangentS)
		StoreElement(VERTEX_ELEMENT_TANGENT_S);
	if (m_Flags & VFF::TangentT)
		StoreElement(VERTEX_ELEMENT_TANGENT_T);
	if (m_Flags & VFF::Wrinkle)
		StoreElement(VERTEX_ELEMENT_WRINKLE);
	if (m_Flags & VFF::BoneIndex)
		StoreElement(VERTEX_ELEMENT_BONEINDEX);

	if (m_BoneWeightCount >= 1)
		StoreElement(VERTEX_ELEMENT_BONEWEIGHTS1);
	if (m_BoneWeightCount >= 2)
		StoreElement(VERTEX_ELEMENT_BONEWEIGHTS2);
	if (m_BoneWeightCount >= 3)
		StoreElement(VERTEX_ELEMENT_BONEWEIGHTS3);
	if (m_BoneWeightCount >= 4)
		StoreElement(VERTEX_ELEMENT_BONEWEIGHTS4);

	switch (m_UserDataSize)
	{
	default:
		assert(!"Unknown userdata size");
	case 0:
		break;
	case 1:
		StoreElement(VERTEX_ELEMENT_USERDATA1);
		break;
	case 2:
		StoreElement(VERTEX_ELEMENT_USERDATA2);
		break;
	case 3:
		StoreElement(VERTEX_ELEMENT_USERDATA3);
		break;
	case 4:
		StoreElement(VERTEX_ELEMENT_USERDATA4);
		break;
	}

	for (int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; i++)
	{
		auto texCoordSize = GetTexCoordSize(i);
		if (texCoordSize > 0)
			StoreElement(VertexElement_t(VERTEX_ELEMENT_TEXCOORD1D_0 + ((texCoordSize - 1) * VERTEX_MAX_TEXTURE_COORDINATES) + i));
	}

	if (totalSize)
		*totalSize = totalSizeTemp;

	return elemCount;
}
