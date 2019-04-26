#include "stdafx.h"
#include "VulkanMesh.h"
#include <TF2Vulkan/Util/FourCC.h>

using namespace TF2Vulkan;

TF2Vulkan::VulkanMesh::VulkanMesh(const VertexFormat& fmt) :
	m_VertexBuffer(fmt)
{
}

void VulkanMesh::SetPrimitiveType(MaterialPrimitiveType_t type)
{
	LOG_FUNC();
	m_PrimitiveType = type;
}

void VulkanMesh::Draw(int firstIndex, int indexCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::SetColorMesh(IMesh* colorMesh, int vertexOffset)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::Draw(CPrimList* lists, int listCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::CopyToMeshBuilder(int startVert, int vertCount, int startIndex, int indexCount, int indexOffset, CMeshBuilder& builder)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::Spew(int vertexCount, int indexCount, const MeshDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::ValidateData(int vertexCount, int indexCount, const MeshDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
{
	LOG_FUNC();
	const auto result1 = m_VertexBuffer.Lock(vertexCount, false, desc);
	const auto result2 = m_IndexBuffer.Lock(indexCount, false, desc);
	assert(result1 && result2);
}

void VulkanMesh::ModifyBegin(int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::ModifyEnd(MeshDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::ModifyBeginEx(bool readOnly, int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::SetFlexMesh(IMesh* mesh, int vertexOffset)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::DisableFlexMesh()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::MarkAsDrawn()
{
	NOT_IMPLEMENTED_FUNC();
}

unsigned VulkanMesh::ComputeMemoryUsed()
{
	return 0;
}

int VulkanMesh::IndexCount() const
{
	LOG_FUNC();
	return m_IndexBuffer.IndexCount();
}

MaterialIndexFormat_t VulkanMesh::IndexFormat() const
{
	LOG_FUNC();
	return m_IndexBuffer.IndexFormat();
}

void VulkanMesh::BeginCastBuffer(MaterialIndexFormat_t format)
{
	LOG_FUNC();
	return m_IndexBuffer.BeginCastBuffer(format);
}

bool VulkanMesh::Lock(int maxIndexCount, bool append, IndexDesc_t& desc)
{
	LOG_FUNC();
	return m_IndexBuffer.Lock(maxIndexCount, append, desc);
}

void VulkanMesh::Unlock(int writtenIndexCount, IndexDesc_t& desc)
{
	LOG_FUNC();
	return m_IndexBuffer.Unlock(writtenIndexCount, desc);
}

void VulkanMesh::ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc)
{
	LOG_FUNC();
	return m_IndexBuffer.ModifyBegin(readOnly, firstIndex, indexCount, desc);
}

void VulkanMesh::ModifyEnd(IndexDesc_t& desc)
{
	LOG_FUNC();
	return m_IndexBuffer.ModifyEnd(desc);
}

void VulkanMesh::Spew(int indexCount, const IndexDesc_t& desc)
{
	LOG_FUNC();
	return m_IndexBuffer.Spew(indexCount, desc);
}

void VulkanMesh::ValidateData(int indexCount, const IndexDesc_t& desc)
{
	LOG_FUNC();
	return m_IndexBuffer.ValidateData(indexCount, desc);
}

int VulkanMesh::VertexCount() const
{
	LOG_FUNC();
	return m_VertexBuffer.VertexCount();
}

VertexFormat_t VulkanMesh::GetVertexFormat() const
{
	LOG_FUNC();
	return m_VertexBuffer.GetVertexFormat();
}

void VulkanMesh::BeginCastBuffer(VertexFormat_t format)
{
	LOG_FUNC();
	return m_VertexBuffer.BeginCastBuffer(format);
}

bool VulkanMesh::Lock(int vertexCount, bool append, VertexDesc_t& desc)
{
	LOG_FUNC();
	return m_VertexBuffer.Lock(vertexCount, append, desc);
}

void VulkanMesh::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC();
	return m_VertexBuffer.Unlock(vertexCount, desc);
}

void VulkanMesh::Spew(int vertexCount, const VertexDesc_t& desc)
{
	LOG_FUNC();
	return m_VertexBuffer.Spew(vertexCount, desc);
}

void VulkanMesh::ValidateData(int vertexCount, const VertexDesc_t& desc)
{
	LOG_FUNC();
	return m_VertexBuffer.ValidateData(vertexCount, desc);
}

bool VulkanMesh::IsDynamic() const
{
	LOG_FUNC();
	assert(!"Ambiguous");

	const bool vtxDyn = m_VertexBuffer.IsDynamic();
	const bool idxDyn = m_IndexBuffer.IsDynamic();
	assert(vtxDyn == idxDyn);
	return vtxDyn || idxDyn;
}

void VulkanMesh::EndCastBuffer()
{
	LOG_FUNC();
	assert(!"Fully ambiguous");
}

int VulkanMesh::GetRoomRemaining() const
{
	LOG_FUNC();

	const auto vtxRoom = m_VertexBuffer.GetRoomRemaining();
	const auto idxRoom = m_IndexBuffer.GetRoomRemaining();

	assert(vtxRoom == idxRoom);
	return min(vtxRoom, idxRoom);
}

int VulkanIndexBuffer::IndexCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

MaterialIndexFormat_t VulkanIndexBuffer::IndexFormat() const
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialIndexFormat_t();
}

bool VulkanIndexBuffer::IsDynamic() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void VulkanIndexBuffer::BeginCastBuffer(MaterialIndexFormat_t format)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanIndexBuffer::EndCastBuffer()
{
	NOT_IMPLEMENTED_FUNC();
}

int VulkanIndexBuffer::GetRoomRemaining() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool VulkanIndexBuffer::Lock(int maxIndexCount, bool append, IndexDesc_t& desc)
{
	LOG_FUNC();

	if (append)
		NOT_IMPLEMENTED_FUNC();

	desc = {};

	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;
	desc.m_nIndexSize = sizeof(m_Indices[0]);

	m_Indices.resize(maxIndexCount);
	desc.m_pIndices = m_Indices.data();

	return true;
}

void VulkanIndexBuffer::Unlock(int writtenIndexCount, IndexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanIndexBuffer::ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanIndexBuffer::ModifyEnd(IndexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanIndexBuffer::Spew(int indexCount, const IndexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanIndexBuffer::ValidateData(int indexCount, const IndexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

VulkanVertexBuffer::VulkanVertexBuffer(const VertexFormat& format) :
	m_Format(format)
{
}

int VulkanVertexBuffer::VertexCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

VertexFormat_t VulkanVertexBuffer::GetVertexFormat() const
{
	NOT_IMPLEMENTED_FUNC();
	return VertexFormat_t();
}

bool VulkanVertexBuffer::IsDynamic() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void VulkanVertexBuffer::BeginCastBuffer(VertexFormat_t format)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanVertexBuffer::EndCastBuffer()
{
	NOT_IMPLEMENTED_FUNC();
}

int VulkanVertexBuffer::GetRoomRemaining() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

template<typename T> void VulkanVertexBuffer::LockStream(VertexFormatFlags flag,
	std::vector<T>& vec, int& descCount, T*& descPtr, int count,
	const Util::FourCC& fallbackPtr)
{
	if (m_Format.m_Flags & flag)
	{
		vec.resize(Util::SafeConvert<size_t>(count));
		descCount = count;
		descPtr = vec.data();
	}
	else
	{
		descCount = 0;
		descPtr = reinterpret_cast<T*>(fallbackPtr.m_Int);
	}
}

bool VulkanVertexBuffer::Lock(int vertexCount, bool append, VertexDesc_t& desc)
{
	LOG_FUNC();

	if (append)
		NOT_IMPLEMENTED_FUNC();

	desc = {};

	static constexpr Util::FourCC POSITION("POSN");
	static constexpr Util::FourCC COLOR("COLR");
	static constexpr Util::FourCC NORMAL("NRML");
	static constexpr Util::FourCC SPECULAR("SPEC");
	static constexpr Util::FourCC TANGENTS("TANS");
	static constexpr Util::FourCC TANGENTT("TANT");
	static constexpr Util::FourCC WRINKLE("WRNK");
	static constexpr Util::FourCC USERDATA("USER");
	static constexpr Util::FourCC TEXCOORD[VERTEX_MAX_TEXTURE_COORDINATES] =
	{
		"TEX0",
		"TEX1",
		"TEX2",
		"TEX3",
		"TEX4",
		"TEX5",
		"TEX6",
		"TEX7",
	};

	LockStream(VertexFormatFlags::Position, m_Position,
		desc.m_VertexSize_Position, desc.m_pPosition, vertexCount, POSITION);

	LockStream(VertexFormatFlags::Color, m_Color,
		desc.m_VertexSize_Color, desc.m_pColor, vertexCount, COLOR);

	LockStream(VertexFormatFlags::Normal, m_Normal,
		desc.m_VertexSize_Normal, desc.m_pNormal, vertexCount, NORMAL);

	LockStream(VertexFormatFlags::Specular, m_Specular,
		desc.m_VertexSize_Specular, desc.m_pSpecular, vertexCount, SPECULAR);

	LockStream(VertexFormatFlags::TangentS, m_TangentS,
		desc.m_VertexSize_TangentS, desc.m_pTangentS, vertexCount, TANGENTS);
	LockStream(VertexFormatFlags::TangentT, m_TangentT,
		desc.m_VertexSize_TangentT, desc.m_pTangentT, vertexCount, TANGENTT);

	LockStream(VertexFormatFlags::Wrinkle, m_Wrinkle,
		desc.m_VertexSize_Wrinkle, desc.m_pWrinkle, vertexCount, WRINKLE);

	for (int i = 0; i < VERTEX_MAX_TEXTURE_COORDINATES; i++)
	{
		auto count = m_Format.GetTexCoordSize(i);
		if (count <= 0)
		{
			desc.m_pTexCoord[i] = reinterpret_cast<float*>(TEXCOORD[i].m_Int);
			continue;
		}

		m_TexCoords[i].resize(count * vertexCount);
		desc.m_pTexCoord[i] = m_TexCoords[i].data();
		desc.m_VertexSize_TexCoord[i] = vertexCount;
	}

	if (m_Format.m_UserDataSize)
	{
		m_UserData.resize(m_Format.m_UserDataSize);
		desc.m_VertexSize_UserData = m_Format.m_UserDataSize;
		desc.m_pUserData = m_UserData.data();
	}
	else
	{
		desc.m_pUserData = reinterpret_cast<float*>(USERDATA.m_Int);
	}

	return true;
}

void VulkanVertexBuffer::Unlock(int vertexCount, VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanVertexBuffer::Spew(int vertexCount, const VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanVertexBuffer::ValidateData(int vertexCount, const VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}
