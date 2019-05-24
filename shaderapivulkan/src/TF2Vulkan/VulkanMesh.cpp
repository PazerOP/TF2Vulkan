#include "IShaderAPI/IShaderAPI_StateManagerDynamic.h"
#include "interface/IMaterialInternal.h"
#include "interface/internal/IBufferPoolInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IStateManagerStatic.h"
#include "VulkanFactories.h"
#include "VulkanMesh.h"
#include <TF2Vulkan/Util/FourCC.h>

using namespace TF2Vulkan;

static std::aligned_storage_t<256> s_FallbackMeshData;

static void AssertCheckHeap()
{
	ENSURE(_CrtCheckMemory());
}

static void ValidateVertexFormat(VertexFormat meshFormat, VertexFormat materialFormat)
{
	if (!materialFormat.IsCompressed())
		assert(!meshFormat.IsCompressed());

	const auto CheckFlag = [&](VertexFormatFlags flag)
	{
		if (meshFormat.m_Flags & flag)
		{
			assert(materialFormat.m_Flags & flag);
		}
	};

	CheckFlag(VertexFormatFlags::Position);
	CheckFlag(VertexFormatFlags::Normal);
	CheckFlag(VertexFormatFlags::Color);
	CheckFlag(VertexFormatFlags::Specular);

	CheckFlag(VertexFormatFlags::TangentS);
	CheckFlag(VertexFormatFlags::TangentT);

	CheckFlag(VertexFormatFlags::Wrinkle);

	// Questionable checks
#if false
	CheckFlag(VertexFormatFlags::BoneIndex);
#endif
}

void VulkanGPUBuffer::UpdateInnerBuffer(const char* dbgName,
	const void* newData, size_t newSize, vk::BufferUsageFlags usage)
{
	if (IsDynamic())
	{
		if (m_Buffer.index() == 0)
			m_Buffer.emplace<BufferPoolEntry>();

		auto& buffer = std::get<BufferPoolEntry>(m_Buffer);

		if (usage & vk::BufferUsageFlagBits::eVertexBuffer)
			buffer = g_ShaderDevice.GetVertexBufferPool().Create(newSize, newData);
		else if (usage & vk::BufferUsageFlagBits::eIndexBuffer)
			buffer = g_ShaderDevice.GetIndexBufferPool().Create(newSize, newData);
		else
		{
			assert(!"Invalid buffer usage flags");
		}
	}
	else
	{
		if (m_Buffer.index() == 0)
			m_Buffer.emplace<vma::AllocatedBuffer>();

		auto& buffer = std::get<vma::AllocatedBuffer>(m_Buffer);

		if (buffer)
			g_ShaderDevice.GetPrimaryCmdBuf().AddResource(std::move(buffer));

		char dbgNameFormatted[128];
		sprintf_s(dbgNameFormatted, "%s(): %s (size %zu)", __FUNCTION__, dbgName, newSize);

		// Recreate
		buffer = Factories::BufferFactory{}
			.SetMemoryType(vma::MemoryType::eCpuToGpu)
			.SetDebugName(dbgNameFormatted)
			.SetInitialData(newData, newSize)
			.SetUsage(usage)
			.Create();
	}
}

VulkanMesh::VulkanMesh(const VertexFormat& fmt, bool isDynamic) :
	m_VertexBuffer(fmt, isDynamic),
	m_IndexBuffer(isDynamic)
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
	AssertCheckHeap();

	if (!g_ShaderDevice.IsPrimaryCmdBufReady())
	{
		Warning(TF2VULKAN_PREFIX "Skipping mesh draw, shader device not ready yet\n");
		return;
	}

	if (firstIndex == -1)
	{
		// "Start at true zero"?
		firstIndex = 0;
	}
	assert(firstIndex >= 0); // Catch other weird values

	assert(indexCount >= 0);
	if (indexCount == 0)
	{
		// Apparently, 0 means "draw everything"
		indexCount = IndexCount();
	}

	ActiveMeshScope meshScope(ActiveMeshData{ this, firstIndex, indexCount });

	auto& dynState = g_StateManagerDynamic.GetDynamicState();
	auto internalMaterial = assert_cast<IMaterialInternal*>(dynState.m_BoundMaterial);
	internalMaterial->DrawMesh(VertexFormat(m_VertexBuffer.GetVertexFormat()).GetCompressionType());
}

void VulkanMesh::DrawInternal(IVulkanCommandBuffer& cmdBuf, int firstIndex, int indexCount)
{
	LOG_FUNC();
	AssertCheckHeap();

	if (!m_VertexBuffer.VertexCount())
	{
		Warning(TF2VULKAN_PREFIX "No vertices\n");
		return;
	}

	vk::Buffer indexBuffer, vertexBuffer;
	size_t indexBufferOffset, vertexBufferOffset;
	m_IndexBuffer.GetGPUBuffer(indexBuffer, indexBufferOffset);
	m_VertexBuffer.GetGPUBuffer(vertexBuffer, vertexBufferOffset);
	cmdBuf.bindIndexBuffer(indexBuffer, indexBufferOffset, vk::IndexType::eUint16);

	// Bind vertex buffers
	{
		const vk::Buffer vtxBufs[] =
		{
			vertexBuffer,
			g_ShaderDevice.GetDummyVertexBuffer(),
		};
		const vk::DeviceSize offsets[] =
		{
			vertexBufferOffset,
			0,
		};
		static_assert(std::size(vtxBufs) == std::size(offsets));
		cmdBuf.bindVertexBuffers(0, TF2Vulkan::to_array_proxy(vtxBufs), TF2Vulkan::to_array_proxy(offsets));
	}

	cmdBuf.drawIndexed(Util::SafeConvert<uint32_t>(indexCount), 1, Util::SafeConvert<uint32_t>(firstIndex));
}

void VulkanMesh::SetColorMesh(IMesh* colorMesh, int vertexOffset)
{
	LOG_FUNC();
	m_ColorMesh = colorMesh;
	m_ColorMeshVertexOffset = vertexOffset;
}

void VulkanMesh::Draw(CPrimList* lists, int listCount)
{
	LOG_FUNC();

	// TODO: Indirect rendering
	for (int i = 0; i < listCount; i++)
		Draw(lists[i].m_FirstIndex, lists[i].m_NumIndices);
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
	LOG_FUNC();
	if (mesh || vertexOffset)
		NOT_IMPLEMENTED_FUNC_NOBREAK(); // TODO: This goes into vPosFlex and vNormalFlex
}

void VulkanMesh::DisableFlexMesh()
{
	LOG_FUNC();
	SetFlexMesh(nullptr, 0);
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

VulkanIndexBuffer::VulkanIndexBuffer(bool isDynamic) :
	VulkanGPUBuffer(isDynamic)
{
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

bool VulkanVertexBuffer::IsDynamic() const
{
	LOG_FUNC();
	return VulkanGPUBuffer::IsDynamic();
}

bool VulkanIndexBuffer::IsDynamic() const
{
	LOG_FUNC();
	return VulkanGPUBuffer::IsDynamic();
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
	AssertCheckHeap();

	if (append)
		NOT_IMPLEMENTED_FUNC();

	desc = {};

	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;
	desc.m_nIndexSize = sizeof(m_Indices[0]) >> 1; // Why?

	m_Indices.resize(maxIndexCount);
	desc.m_pIndices = m_Indices.data();

	return true;
}

void VulkanIndexBuffer::Unlock(int writtenIndexCount, IndexDesc_t& desc)
{
	LOG_FUNC();
	AssertCheckHeap();
	assert(Util::SafeConvert<size_t>(writtenIndexCount) <= (m_Indices.size() * sizeof(m_Indices[0])));

	UpdateInnerBuffer("VulkanIndexBuffer", IndexData(), writtenIndexCount * sizeof(m_Indices[0]),
		vk::BufferUsageFlagBits::eIndexBuffer);
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

VulkanGPUBuffer::VulkanGPUBuffer(bool isDynamic) :
	m_IsDynamic(isDynamic)
{
}

void VulkanGPUBuffer::GetGPUBuffer(vk::Buffer& buffer, size_t& offset) const
{
	if (auto foundBuf = std::get_if<vma::AllocatedBuffer>(&m_Buffer))
	{
		buffer = foundBuf->GetBuffer();
		offset = 0;
	}
	else if (auto foundBuf = std::get_if<BufferPoolEntry>(&m_Buffer))
	{
		offset = foundBuf->GetOffset();
		buffer = static_cast<const IBufferPoolInternal&>(foundBuf->GetPool()).GetBuffer(offset);
	}
	else
	{
		throw VulkanException("Invalid buffer state", EXCEPTION_DATA());
	}
}

const unsigned short* VulkanIndexBuffer::IndexData() const
{
	return m_Indices.data();
}

size_t VulkanIndexBuffer::IndexDataSize() const
{
	return m_Indices.size() * sizeof(m_Indices[0]);
}

VulkanVertexBuffer::VulkanVertexBuffer(const VertexFormat& format, bool isDynamic) :
	VulkanGPUBuffer(isDynamic),
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
	LOG_FUNC();
	return m_Format;
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
	AssertCheckHeap();

	assert(!m_Format.IsUnknownFormat());

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
	//uncompressedFormat.SetCompressionEnabled(false);

	desc.m_NumBoneWeights = uncompressedFormat.m_BoneWeightCount;

	const auto vtxElemsCount = uncompressedFormat.GetVertexElements(vtxElems, std::size(vtxElems), &totalVtxSize);

	Util::SafeConvert(vertexCount, m_VertexCount);
	m_DataBuffer.resize(totalVtxSize * m_VertexCount);
	assert(!m_DataBuffer.empty());

	Util::SafeConvert(totalVtxSize, desc.m_ActualVertexSize);
	desc.m_CompressionType = uncompressedFormat.GetCompressionType();

	for (uint_fast8_t i = 0; i < vtxElemsCount; i++)
	{
		const auto& vtxElem = vtxElems[i];

		const auto UpdateElementParams = [&](auto & sizeParam, auto*& dataPtrParam)
		{
			using ptrType = std::decay_t<decltype(dataPtrParam)>;
			Util::SafeConvert(totalVtxSize, sizeParam);
			dataPtrParam = reinterpret_cast<ptrType>(m_DataBuffer.data() + vtxElem.m_Offset);
		};

		switch (vtxElem.m_Type->m_Element)
		{
		default:
			assert(!"Unknown vertex element type");
		case VERTEX_ELEMENT_NONE:
			break;

		case VERTEX_ELEMENT_TEXCOORD1D_0:
		case VERTEX_ELEMENT_TEXCOORD1D_1:
		case VERTEX_ELEMENT_TEXCOORD1D_2:
		case VERTEX_ELEMENT_TEXCOORD1D_3:
		case VERTEX_ELEMENT_TEXCOORD1D_4:
		case VERTEX_ELEMENT_TEXCOORD1D_5:
		case VERTEX_ELEMENT_TEXCOORD1D_6:
		case VERTEX_ELEMENT_TEXCOORD1D_7:
		case VERTEX_ELEMENT_TEXCOORD2D_0:
		case VERTEX_ELEMENT_TEXCOORD2D_1:
		case VERTEX_ELEMENT_TEXCOORD2D_2:
		case VERTEX_ELEMENT_TEXCOORD2D_3:
		case VERTEX_ELEMENT_TEXCOORD2D_4:
		case VERTEX_ELEMENT_TEXCOORD2D_5:
		case VERTEX_ELEMENT_TEXCOORD2D_6:
		case VERTEX_ELEMENT_TEXCOORD2D_7:
		case VERTEX_ELEMENT_TEXCOORD3D_0:
		case VERTEX_ELEMENT_TEXCOORD3D_1:
		case VERTEX_ELEMENT_TEXCOORD3D_2:
		case VERTEX_ELEMENT_TEXCOORD3D_3:
		case VERTEX_ELEMENT_TEXCOORD3D_4:
		case VERTEX_ELEMENT_TEXCOORD3D_5:
		case VERTEX_ELEMENT_TEXCOORD3D_6:
		case VERTEX_ELEMENT_TEXCOORD3D_7:
		case VERTEX_ELEMENT_TEXCOORD4D_0:
		case VERTEX_ELEMENT_TEXCOORD4D_1:
		case VERTEX_ELEMENT_TEXCOORD4D_2:
		case VERTEX_ELEMENT_TEXCOORD4D_3:
		case VERTEX_ELEMENT_TEXCOORD4D_4:
		case VERTEX_ELEMENT_TEXCOORD4D_5:
		case VERTEX_ELEMENT_TEXCOORD4D_6:
		case VERTEX_ELEMENT_TEXCOORD4D_7:
		{
			const auto texCoordIdx = (vtxElem.m_Type->m_Element - VERTEX_ELEMENT_TEXCOORD1D_0) % VERTEX_MAX_TEXTURE_COORDINATES;
			UpdateElementParams(desc.m_VertexSize_TexCoord[texCoordIdx], desc.m_pTexCoord[texCoordIdx]);
			break;
		}

		case VERTEX_ELEMENT_BONEINDEX:
			UpdateElementParams(desc.m_VertexSize_BoneMatrixIndex, desc.m_pBoneMatrixIndex);
			break;

		case VERTEX_ELEMENT_BONEWEIGHTS1:
		case VERTEX_ELEMENT_BONEWEIGHTS2:
		case VERTEX_ELEMENT_BONEWEIGHTS3:
		case VERTEX_ELEMENT_BONEWEIGHTS4:
			UpdateElementParams(desc.m_VertexSize_BoneWeight, desc.m_pBoneWeight);
			break;

		case VERTEX_ELEMENT_USERDATA1:
		case VERTEX_ELEMENT_USERDATA2:
		case VERTEX_ELEMENT_USERDATA3:
		case VERTEX_ELEMENT_USERDATA4:
			UpdateElementParams(desc.m_VertexSize_UserData, desc.m_pUserData);
			break;

		case VERTEX_ELEMENT_POSITION:
			UpdateElementParams(desc.m_VertexSize_Position, desc.m_pPosition);
			break;
		case VERTEX_ELEMENT_NORMAL:
			UpdateElementParams(desc.m_VertexSize_Normal, desc.m_pNormal);
			break;
		case VERTEX_ELEMENT_COLOR:
			UpdateElementParams(desc.m_VertexSize_Color, desc.m_pColor);
			break;
		case VERTEX_ELEMENT_SPECULAR:
			UpdateElementParams(desc.m_VertexSize_Specular, desc.m_pSpecular);
			break;
		case VERTEX_ELEMENT_TANGENT_S:
			UpdateElementParams(desc.m_VertexSize_TangentS, desc.m_pTangentS);
			break;
		case VERTEX_ELEMENT_TANGENT_T:
			UpdateElementParams(desc.m_VertexSize_TangentT, desc.m_pTangentT);
			break;
		case VERTEX_ELEMENT_WRINKLE:
			UpdateElementParams(desc.m_VertexSize_Wrinkle, desc.m_pWrinkle);
			break;
		}
	}

	return true;
}

void VulkanVertexBuffer::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC();
	AssertCheckHeap();
	ValidateData(vertexCount, desc);
	assert(Util::SafeConvert<size_t>(vertexCount) <= m_VertexCount);

	UpdateInnerBuffer("VulkanVertexBuffer", VertexData(),
		(VertexDataSize() / m_VertexCount) * vertexCount, vk::BufferUsageFlagBits::eVertexBuffer);
}

void VulkanVertexBuffer::Spew(int vertexCount, const VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

template<typename T>
static void ValidateType(const void* base, int elementIndex, int elementSize)
{
	if (elementSize <= 0)
		return;

	const T& typed = *reinterpret_cast<const T*>(reinterpret_cast<const std::byte*>(base) + elementSize * elementIndex);
	using namespace Shaders;
	if constexpr (std::is_same_v<T, float1> || std::is_same_v<T, float2> || std::is_same_v<T, float3> || std::is_same_v<T, float4>)
	{
		if constexpr (T::ELEM_COUNT >= 1)
			assert(std::isfinite(typed.x));
		if constexpr (T::ELEM_COUNT >= 2)
			assert(std::isfinite(typed.y));
		if constexpr (T::ELEM_COUNT >= 3)
			assert(std::isfinite(typed.z));
		if constexpr (T::ELEM_COUNT >= 4)
			assert(std::isfinite(typed.w));
	}
}

void VulkanVertexBuffer::ValidateData(int vertexCount, const VertexDesc_t& desc)
{
	LOG_FUNC();

	for (int i = 0; i < vertexCount; i++)
	{
		ValidateType<Shaders::float3>(desc.m_pPosition, i, desc.m_VertexSize_Position);
	}
}

const std::byte* VulkanVertexBuffer::VertexData() const
{
	return m_DataBuffer.data();
}

size_t VulkanVertexBuffer::VertexDataSize() const
{
	return m_DataBuffer.size() * sizeof(m_DataBuffer[0]);
}
