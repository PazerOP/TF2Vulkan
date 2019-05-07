#include "IStateManagerDynamic.h"
#include "interface/IMaterialInternal.h"
#include "ShaderDevice.h"
#include "interface/internal/IStateManagerStatic.h"
#include "VulkanFactories.h"
#include "VulkanMesh.h"
#include <TF2Vulkan/Util/FourCC.h>

using namespace TF2Vulkan;

static std::aligned_storage_t<256> s_FallbackMeshData;

VulkanMesh::VulkanMesh(const VertexFormat& fmt) :
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
	LOG_FUNC();

	if (!g_ShaderDevice.IsReady())
	{
		Warning(TF2VULKAN_PREFIX "Skipping mesh draw, shader device not ready yet\n");
		return;
	}

	assert(firstIndex == -1); // TODO: What about other values
	if (firstIndex == -1)
	{
		// "Start at true zero"?
		firstIndex = 0;
	}

	assert(indexCount >= 0);
	if (indexCount == 0)
	{
		// Apparently, 0 means "draw everything"
		indexCount = IndexCount();
	}

	ActiveMeshScope meshScope(ActiveMeshData{ this, firstIndex, indexCount });

	auto& dynState = g_StateManagerDynamic.GetDynamicState();
	auto internalMaterial = assert_cast<IMaterialInternal*>(dynState.m_BoundMaterial);
	internalMaterial->DrawMesh(VertexCompressionType_t::VERTEX_COMPRESSION_ON);
}

void VulkanMesh::DrawInternal(IVulkanCommandBuffer& cmdBuf, int firstIndex, int indexCount)
{
	LOG_FUNC();

	auto pixScope = cmdBuf.DebugRegionBegin(Color(128, 255, 128), "VulkanMesh::DrawInternal()");

	auto indexBuf = Factories::BufferFactory{}
		.SetUsage(vk::BufferUsageFlagBits::eIndexBuffer)
		.SetInitialData(m_IndexBuffer.IndexData(), m_IndexBuffer.IndexDataSize() * IndexCount())
		.SetDebugName(__FUNCTION__ "(): Test index buffer")
		.Create();

	auto vertexBuf = Factories::BufferFactory{}
		.SetUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.SetInitialData(m_VertexBuffer.VertexData(), m_VertexBuffer.VertexDataSize() * VertexCount())
		.SetDebugName(__FUNCTION__ "(): Test vertex buffer")
		.Create();

	auto dummyVertexBuf = Factories::BufferFactory{}
		.SetUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.SetSize(sizeof(s_FallbackMeshData))
		.SetDebugName(__FUNCTION__ "(): Dummy vertex buffer (unused attributes)")
		.Create();

	cmdBuf.bindIndexBuffer(indexBuf.GetBuffer(), 0, vk::IndexType::eUint16);
	cmdBuf.AddResource(std::move(indexBuf));

	// Bind vertex buffers
	{
		const vk::Buffer vtxBufs[] =
		{
			vertexBuf.GetBuffer(),
			dummyVertexBuf.GetBuffer(),
		};
		const vk::DeviceSize offsets[] =
		{
			0,
			0,
		};
		static_assert(std::size(vtxBufs) == std::size(offsets));
		cmdBuf.bindVertexBuffers(0, TF2Vulkan::to_array_proxy(vtxBufs), TF2Vulkan::to_array_proxy(offsets));
		cmdBuf.AddResource(std::move(vertexBuf));
		cmdBuf.AddResource(std::move(dummyVertexBuf));
	}

	assert(firstIndex == 0); // TODO: What happens when we actually have offsets?
	cmdBuf.drawIndexed(Util::SafeConvert<uint32_t>(indexCount));
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
	LOG_FUNC();
	m_VertexBuffer.Unlock(vertexCount, desc);
	m_IndexBuffer.Unlock(indexCount, desc);
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
	NOT_IMPLEMENTED_FUNC();
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
	LOG_FUNC();
	return m_Indices.size();
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
	LOG_FUNC();
	assert(Util::SafeConvert<size_t>(writtenIndexCount) <= (m_Indices.size() * sizeof(m_Indices[0])));
	// TODO: Send data to GPU
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

const unsigned short* VulkanIndexBuffer::IndexData() const
{
	return m_Indices.data();
}

size_t VulkanIndexBuffer::IndexDataSize() const
{
	return m_Indices.size() * sizeof(m_Indices[0]);
}

VulkanVertexBuffer::VulkanVertexBuffer(const VertexFormat& format) :
	m_Format(format)
{
}

int VulkanVertexBuffer::VertexCount() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(m_VertexCount);
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

bool VulkanVertexBuffer::Lock(int vertexCount, bool append, VertexDesc_t& desc)
{
	LOG_FUNC();

	if (append)
		NOT_IMPLEMENTED_FUNC();

	desc = {};

	// Set default dummy pointers
	desc.m_pPosition = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pBoneWeight = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pBoneMatrixIndex = reinterpret_cast<unsigned char*>(&s_FallbackMeshData);
	desc.m_pNormal = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pColor = reinterpret_cast<unsigned char*>(&s_FallbackMeshData);
	desc.m_pSpecular = reinterpret_cast<unsigned char*>(&s_FallbackMeshData);
	desc.m_pTangentS = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pTangentT = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pWrinkle = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pUserData = reinterpret_cast<float*>(&s_FallbackMeshData);
	for (auto& tc : desc.m_pTexCoord)
		tc = reinterpret_cast<float*>(&s_FallbackMeshData);

	VertexFormat::Element vtxElems[VERTEX_ELEMENT_NUMELEMENTS];
	size_t totalVtxSize;

	auto uncompressedFormat = m_Format;
	uncompressedFormat.SetCompressionEnabled(false);

	const auto vtxElemsCount = uncompressedFormat.GetVertexElements(vtxElems, std::size(vtxElems), &totalVtxSize);

	Util::SafeConvert(vertexCount, m_VertexCount);
	m_DataBuffer.resize(totalVtxSize * m_VertexCount);

	Util::SafeConvert(totalVtxSize, desc.m_ActualVertexSize);
	desc.m_CompressionType = uncompressedFormat.GetCompressionType();

	for (uint_fast8_t i = 0; i < vtxElemsCount; i++)
	{
		const auto& vtxElem = vtxElems[i];
		switch (vtxElem.m_Type->m_Element)
		{
		default:
			if (vtxElem.m_Type->m_Element >= VERTEX_ELEMENT_TEXCOORD1D_0 && vtxElem.m_Type->m_Element < VERTEX_ELEMENT_NUMELEMENTS)
			{
				const auto texCoordIdx = (vtxElem.m_Type->m_Element - VERTEX_ELEMENT_TEXCOORD1D_0) % VERTEX_MAX_TEXTURE_COORDINATES;
				desc.m_VertexSize_TexCoord[texCoordIdx] = totalVtxSize;
				desc.m_pTexCoord[texCoordIdx] = reinterpret_cast<float*>(m_DataBuffer.data() + vtxElem.m_Offset);

				break;
			}

			assert(!"Unknown vertex element type");
		case VERTEX_ELEMENT_NONE:
			break;

		case VERTEX_ELEMENT_POSITION:
			Util::SafeConvert(totalVtxSize, desc.m_VertexSize_Position);
			desc.m_pPosition = reinterpret_cast<float*>(m_DataBuffer.data() + vtxElem.m_Offset);
			break;
		case VERTEX_ELEMENT_NORMAL:
			Util::SafeConvert(totalVtxSize, desc.m_VertexSize_Normal);
			desc.m_pNormal = reinterpret_cast<float*>(m_DataBuffer.data() + vtxElem.m_Offset);
			break;
		}
	}

	return true;
}

void VulkanVertexBuffer::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC();
	// TODO: Send data to GPU
}

void VulkanVertexBuffer::Spew(int vertexCount, const VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanVertexBuffer::ValidateData(int vertexCount, const VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

const std::byte* VulkanVertexBuffer::VertexData() const
{
	return m_DataBuffer.data();
}

size_t VulkanVertexBuffer::VertexDataSize() const
{
	static_assert(sizeof(m_DataBuffer[0]) == 1);
	return m_DataBuffer.size();
}
