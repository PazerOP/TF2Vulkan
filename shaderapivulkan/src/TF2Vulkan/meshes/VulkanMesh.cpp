#include "TF2Vulkan/IShaderAPI/IShaderAPI_StateManagerDynamic.h"
#include "interface/IMaterialInternal.h"
#include "interface/internal/IBufferPoolInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IStateManagerStatic.h"
#include "TF2Vulkan/VulkanFactories.h"
#include "VulkanMesh.h"

#include <TF2Vulkan/Util/FourCC.h>
#include <TF2Vulkan/Util/std_variant.h>

using namespace TF2Vulkan;

void VulkanGPUBuffer::AssertCheckHeap()
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

auto VulkanGPUBuffer::GetBufferData(size_t size, size_t offset, bool read, bool write) -> ModifyData
{
	AssertHasLock();
	LOG_FUNC_ANYTHREAD();

	ModifyData retVal;
	retVal.m_DataLength = size;
	retVal.m_DataOffset = offset;

	if (size <= 0)
	{
		retVal.m_Data = &IShaderAPI_MeshManager::s_FallbackMeshData;
		return retVal;
	}

	assert(read || write);

	const auto actualMinSize = IsDynamic() ? size + offset : size;

	BufferPoolEntry buffer;

	/*if (read && write)
	{
		NOT_IMPLEMENTED_FUNC();
	}
	else*/ if (write)
	{
		if (read)
			NOT_IMPLEMENTED_FUNC_NOBREAK();

		const auto usage = IsDynamic() ? m_Usage : vk::BufferUsageFlagBits::eTransferSrc;
		buffer = g_ShaderDevice.GetBufferPool(usage).Create(actualMinSize);
	}
	else if (read)
	{
		NOT_IMPLEMENTED_FUNC();
	}

	auto& realPool = static_cast<IBufferPoolInternal&>(buffer.GetPool());
	auto entryBuffer = realPool.GetBufferInfo(buffer);
	retVal.m_Data = realPool.GetBufferData(buffer.GetOffset()) + (IsDynamic() ? offset : 0);

	if (IsDynamic())
		m_Buffer.emplace<BufferPoolEntry>(buffer);
	else
		retVal.m_PoolEntry = buffer;

	return retVal;
}

void VulkanGPUBuffer::CommitModifications(const VulkanGPUBuffer::ModifyData& modification)
{
	AssertHasLock();
	LOG_FUNC_ANYTHREAD();

	if (modification.m_DataLength <= 0)
		return; // Nothing to do

	if (IsDynamic())
		return; // We write directly into buffer memory, nothing else to do

	assert(std::holds_alternative<std::monostate>(m_Buffer) || std::holds_alternative<vma::AllocatedBuffer>(m_Buffer));

	const auto& entry = modification.m_PoolEntry;
	assert(entry);

	const auto& realPool = static_cast<const IBufferPoolInternal&>(entry.GetPool());
	const auto entryInfo = realPool.GetBufferInfo(entry);

	auto& cmdBuf = g_ShaderDevice.GetPrimaryCmdBuf();

	const auto minSize = modification.m_DataOffset + modification.m_DataLength;
	auto& dstBuf = Util::get_or_emplace<vma::AllocatedBuffer>(m_Buffer);
	if (!dstBuf || dstBuf.size() < minSize)
	{
		if (dstBuf)
			cmdBuf.AddResource(std::move(dstBuf));

		dstBuf = Factories::BufferFactory{}
			.SetSize(minSize)
			.SetUsage(m_Usage | vk::BufferUsageFlagBits::eTransferDst)
			.SetMemoryType(vma::MemoryType::eGpuOnly)
			.Create();
	}

	const auto region = vk::BufferCopy{}
		.setSize(modification.m_DataLength)
		.setSrcOffset(entry.GetOffset())
		.setDstOffset(modification.m_DataOffset);


	if (auto [transfer, lock] = g_ShaderDevice.GetTransferQueue().locked(); transfer)
	{
		auto transferCmdBuf = transfer->CreateCmdBufferAndBegin();
		transferCmdBuf->copyBuffer(entryInfo.m_Buffer, dstBuf.GetBuffer(), region);
		transferCmdBuf->Submit();
	}
	else
	{
		cmdBuf.TryEndRenderPass();
		cmdBuf.copyBuffer(entryInfo.m_Buffer, dstBuf.GetBuffer(), region);
	}
}

VulkanMesh::VulkanMesh(const VertexFormat& fmt, bool isDynamic) :
	m_VertexBuffer(fmt, isDynamic),
	m_IndexBuffer(isDynamic)
{
}

void VulkanMesh::SetPrimitiveType(MaterialPrimitiveType_t type)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	m_PrimitiveType = type;
}

class CMaterial_QueueFriendly : public IMaterialInternal
{
	// Test
};

void VulkanMesh::Draw(int firstIndex, int indexCount)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC();

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
		if (indexCount <= 0)
			return; // Nothing to draw
	}

	ActiveMeshScope meshScope(ActiveMeshData{ this, firstIndex, indexCount });

	auto& dynState = g_StateManagerDynamic.GetDynamicState();
	auto internalMaterial = assert_cast<IMaterialInternal*>(dynState.m_BoundMaterial);
#ifdef _DEBUG
	[[maybe_unused]] auto matName = internalMaterial->GetName();
	[[maybe_unused]] auto shader = internalMaterial->GetShader();
	[[maybe_unused]] auto shaderName = internalMaterial->GetShaderName();
	auto matQueueFriendly = dynamic_cast<CMaterial_QueueFriendly*>(internalMaterial);
	if (matQueueFriendly)
		__debugbreak();
#endif
	internalMaterial->DrawMesh(VertexFormat(GetVertexFormat()).GetCompressionType());
}

void VulkanMesh::DrawInternal(IVulkanCommandBuffer& cmdBuf, int firstIndex, int indexCount)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC();

	if (!VertexCount())
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
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	m_ColorMesh = colorMesh;
	m_ColorMeshVertexOffset = vertexOffset;
}

void VulkanMesh::Draw(CPrimList* lists, int listCount)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC();

	// TODO: Indirect rendering?
	for (int i = 0; i < listCount; i++)
		Draw(lists[i].m_FirstIndex, lists[i].m_NumIndices);
}

void VulkanMesh::CopyToMeshBuilder(int startVert, int vertCount, int startIndex, int indexCount, int indexOffset, CMeshBuilder& builder)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMesh::Spew(int vertexCount, int indexCount, const MeshDesc_t& desc)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	Spew(vertexCount, static_cast<const VertexDesc_t&>(desc));
	Spew(vertexCount, static_cast<const IndexDesc_t&>(desc));
}

void VulkanMesh::ValidateData(int vertexCount, int indexCount, const MeshDesc_t& desc)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	ValidateData(vertexCount, static_cast<const VertexDesc_t&>(desc));
	ValidateData(indexCount, static_cast<const IndexDesc_t&>(desc));
}

void VulkanMesh::LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	const auto result1 = Lock(vertexCount, false, static_cast<VertexDesc_t&>(desc));
	const auto result2 = Lock(indexCount, false, static_cast<IndexDesc_t&>(desc));
	assert(result1 && result2);
}

void VulkanMesh::ModifyBegin(int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return ModifyBeginEx(false, firstVertex, vertexCount, firstIndex, indexCount, desc);
}

void VulkanMesh::ModifyEnd(MeshDesc_t& desc)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	m_VertexBuffer.ModifyEnd(desc);
	ModifyEnd(static_cast<IndexDesc_t&>(desc));
}

void VulkanMesh::UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	Unlock(vertexCount, static_cast<VertexDesc_t&>(desc));
	Unlock(indexCount, static_cast<IndexDesc_t&>(desc));
}

void VulkanMesh::ModifyBeginEx(bool readOnly, int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	m_VertexBuffer.ModifyBegin(readOnly, firstVertex, vertexCount, desc);
	ModifyBegin(readOnly, firstIndex, indexCount, static_cast<IndexDesc_t&>(desc));
}

void VulkanMesh::SetFlexMesh(IMesh* mesh, int vertexOffset)
{
	LOG_FUNC();
	if (mesh || vertexOffset)
		NOT_IMPLEMENTED_FUNC_NOBREAK(); // TODO: This goes into vPosFlex and vNormalFlex
}

void VulkanMesh::DisableFlexMesh()
{
	LOG_FUNC_ANYTHREAD();
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
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.IndexCount();
}

MaterialIndexFormat_t VulkanMesh::IndexFormat() const
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.IndexFormat();
}

void VulkanMesh::BeginCastBuffer(MaterialIndexFormat_t format)
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.BeginCastBuffer(format);
}

bool VulkanMesh::Lock(int maxIndexCount, bool append, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.Lock(maxIndexCount, append, desc);
}

void VulkanMesh::Unlock(int writtenIndexCount, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.Unlock(writtenIndexCount, desc);
}

void VulkanMesh::ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.ModifyBegin(readOnly, firstIndex, indexCount, desc);
}

void VulkanMesh::ModifyEnd(IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	m_IndexBuffer.ModifyEnd(desc);
}

void VulkanMesh::Spew(int indexCount, const IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.Spew(indexCount, desc);
}

void VulkanMesh::ValidateData(int indexCount, const IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_IndexBuffer.ValidateData(indexCount, desc);
}

int VulkanMesh::VertexCount() const
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.VertexCount();
}

VertexFormat_t VulkanMesh::GetVertexFormat() const
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.GetVertexFormat();
}

void VulkanMesh::BeginCastBuffer(VertexFormat_t format)
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.BeginCastBuffer(format);
}

bool VulkanMesh::Lock(int vertexCount, bool append, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.Lock(vertexCount, append, desc);
}

void VulkanMesh::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.Unlock(vertexCount, desc);
}

void VulkanMesh::Spew(int vertexCount, const VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.Spew(vertexCount, desc);
}

void VulkanMesh::ValidateData(int vertexCount, const VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return m_VertexBuffer.ValidateData(vertexCount, desc);
}

bool VulkanMesh::IsDynamic() const
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	assert(!"Ambiguous");

	const bool vtxDyn = m_VertexBuffer.IsDynamic();
	const bool idxDyn = m_IndexBuffer.IsDynamic();
	assert(vtxDyn == idxDyn);
	return vtxDyn || idxDyn;
}

void VulkanMesh::EndCastBuffer()
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	m_VertexBuffer.EndCastBuffer();
	m_IndexBuffer.EndCastBuffer();
}

int VulkanMesh::GetRoomRemaining() const
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();

	const auto vtxRoom = m_VertexBuffer.GetRoomRemaining();
	const auto idxRoom = m_IndexBuffer.GetRoomRemaining();

	assert(vtxRoom == idxRoom);
	return Util::algorithm::min(vtxRoom, idxRoom);
}

VulkanIndexBuffer::VulkanIndexBuffer(bool isDynamic) :
	VulkanGPUBuffer(isDynamic, vk::BufferUsageFlagBits::eIndexBuffer)
{
}

int VulkanIndexBuffer::IndexCount() const
{
	auto lock = ScopeThreadLock();
	LOG_FUNC();
	return Util::SafeConvert<int>(m_TotalIndexCount);
}

MaterialIndexFormat_t VulkanIndexBuffer::IndexFormat() const
{
	LOG_FUNC_ANYTHREAD();
	return MATERIAL_INDEX_FORMAT;
}

bool VulkanVertexBuffer::IsDynamic() const
{
	LOG_FUNC_ANYTHREAD();
	return VulkanGPUBuffer::IsDynamic();
}

bool VulkanIndexBuffer::IsDynamic() const
{
	LOG_FUNC_ANYTHREAD();
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

void VulkanIndexBuffer::ModifyBegin(int firstIndex, int indexCount, IndexDesc_t& desc, bool read, bool write)
{
	LOG_FUNC_ANYTHREAD();
	return ModifyBegin(Util::SafeConvert<uint32_t>(firstIndex), Util::SafeConvert<uint32_t>(indexCount),
		desc, read, write);
}

void VulkanIndexBuffer::ModifyBegin(uint32_t firstIndex, uint32_t indexCount, IndexDesc_t& desc, bool read, bool write)
{
	BeginThreadLock();
	LOG_FUNC_ANYTHREAD();
	assert(!m_ModifyData);

	desc = {};
	desc.m_nIndexSize = sizeof(IndexFormatType) >> 1; // Why?

	if (indexCount <= 0)
	{
		desc.m_pIndices = reinterpret_cast<IndexFormatType*>(&IShaderAPI_MeshManager::s_FallbackMeshData);
		auto& modify = m_ModifyData.emplace();
		modify.m_Data = nullptr;
		modify.m_DataLength = 0;
		modify.m_DataOffset = 0;

		return;
	}

	auto& modifyData = m_ModifyData.emplace(GetBufferData(
		indexCount * IndexElementSize(), firstIndex * IndexElementSize(), read, write));

	desc.m_pIndices = reinterpret_cast<IndexFormatType*>(modifyData.m_Data);
	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;

	assert(desc.m_pIndices);
}

void VulkanIndexBuffer::ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	const bool shouldAllowRead = true;
	const bool shouldAllowWrite = !readOnly;
	return ModifyBegin(firstIndex, indexCount, desc, shouldAllowRead, shouldAllowWrite);
}

void VulkanIndexBuffer::ModifyEnd(IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	AssertCheckHeap();

	auto& modify = m_ModifyData.value();
	if (modify.m_Data && modify.m_DataLength > 0)
	{
		CommitModifications(modify);

		if (auto newIdxCount = (modify.m_DataOffset + modify.m_DataLength) / IndexElementSize(); newIdxCount > m_TotalIndexCount)
			Util::SafeConvert(newIdxCount, m_TotalIndexCount);

		ValidateData(modify.m_DataLength, desc);
	}

	m_ModifyData.reset();

	EndThreadLock();
}

void VulkanIndexBuffer::Spew(int indexCount, const IndexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanIndexBuffer::ValidateData(int indexCount, const IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return ValidateData(Util::SafeConvert<uint32_t>(indexCount), 0, desc);
}

void VulkanIndexBuffer::ValidateData(uint32_t indexCount, uint32_t firstIndex, const IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	// TODO
}

VulkanGPUBuffer::VulkanGPUBuffer(bool isDynamic, vk::BufferUsageFlags usage) :
	m_IsDynamic(isDynamic),
	m_Usage(usage)
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
		const auto& realPool = static_cast<const IBufferPoolInternal&>(g_ShaderDevice.GetBufferPool(m_Usage));
		buffer = realPool.GetBufferInfo(0).m_Buffer;
		offset = 0;
	}
}

VulkanVertexBuffer::VulkanVertexBuffer(const VertexFormat& format, bool isDynamic) :
	VulkanGPUBuffer(isDynamic, vk::BufferUsageFlagBits::eVertexBuffer),
	m_Format(format)
{
}

int VulkanVertexBuffer::VertexCount() const
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
	return Util::SafeConvert<int>(m_TotalVertexCount);
}

VertexFormat_t VulkanVertexBuffer::GetVertexFormat() const
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();
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
	LOG_FUNC_ANYTHREAD();
	assert(vertexCount >= 0);
	ModifyBegin(append ? VertexCount() : 0, vertexCount, desc, false, true);
	return true;
}

bool VulkanIndexBuffer::Lock(int indexCount, bool append, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	assert(indexCount >= 0);
	ModifyBegin(append ? IndexCount() : 0, indexCount, desc, false, true);
	return true;
}

void VulkanVertexBuffer::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	auto& modify = m_ModifyData.value();
	assert(Util::SafeConvert<size_t>(vertexCount) <= modify.m_VertexCount);
	Util::SafeConvert(vertexCount, modify.m_VertexCount);
	Util::SafeConvert(modify.m_VertexCount, m_TotalVertexCount);

	ModifyEnd(desc);
}

void VulkanIndexBuffer::Unlock(int indexCount, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	AssertCheckHeap();

	auto& modify = m_ModifyData.value();
	assert(Util::SafeConvert<size_t>(indexCount) <= (modify.m_DataLength / IndexElementSize()));
	Util::SafeConvert(indexCount, m_TotalIndexCount);
	Util::SafeConvert(m_TotalIndexCount * IndexElementSize(), modify.m_DataLength);

	ModifyEnd(desc);
}

void VulkanVertexBuffer::ModifyEnd(VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	const auto& modify = m_ModifyData.value();
	CommitModifications(modify);

	if (modify.m_VertexCount > m_TotalVertexCount)
		Util::SafeConvert(modify.m_VertexCount, m_TotalVertexCount);

	m_ModifyData.reset();

	EndThreadLock();
}

void VulkanVertexBuffer::ModifyBegin(int firstVertex, int vertexCount, VertexDesc_t& desc, bool read, bool write)
{
	LOG_FUNC_ANYTHREAD();
	return ModifyBegin(Util::SafeConvert<uint32_t>(firstVertex), Util::SafeConvert<uint32_t>(vertexCount),
		desc, read, write);
}

void VulkanVertexBuffer::ModifyBegin(uint32_t firstVertex, uint32_t vertexCount, VertexDesc_t& desc, bool read, bool write)
{
	BeginThreadLock();
	LOG_FUNC_ANYTHREAD();
	AssertCheckHeap();

	assert(!m_Format.IsUnknownFormat());

	if (read)
		NOT_IMPLEMENTED_FUNC_NOBREAK();

	VertexFormat::Element vtxElems[VERTEX_ELEMENT_NUMELEMENTS];
	size_t totalVtxSize;

	desc.m_NumBoneWeights = m_Format.m_BoneWeightCount;

	const auto vtxElemsCount = m_Format.GetVertexElements(vtxElems, std::size(vtxElems), &totalVtxSize);

	assert(!m_ModifyData);
	auto& modify = m_ModifyData.emplace(GetBufferData(vertexCount * totalVtxSize, firstVertex * totalVtxSize, read, write));
	Util::SafeConvert(vertexCount, modify.m_VertexCount);

	g_MeshManager.ComputeVertexDescription(modify.m_Data, m_Format, desc);
}

void VulkanVertexBuffer::ModifyBegin(bool readOnly, int firstVertex, int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	return ModifyBegin(firstVertex, vertexCount, desc, true, !readOnly);
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
	LOG_FUNC_ANYTHREAD();
	return ValidateData(Util::SafeConvert<uint32_t>(vertexCount), 0, desc);
}

void VulkanVertexBuffer::ValidateData(uint32_t vertexCount, uint32_t firstVertex, const VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	const uint32_t endVertex = firstVertex + vertexCount;
	for (uint32_t i = firstVertex; i < endVertex; i++)
	{
		ValidateType<Shaders::float3>(desc.m_pPosition, i, desc.m_VertexSize_Position);

		if (desc.m_CompressionType == VERTEX_COMPRESSION_NONE)
			ValidateType<Shaders::float3>(desc.m_pNormal, i, desc.m_VertexSize_Normal);
	}
}

auto VulkanMeshCommon::ScopeThreadLock() const -> std::unique_lock<MutexType>
{
	return std::unique_lock(m_Mutex);
}

void VulkanMeshCommon::BeginThreadLock() const
{
	m_Mutex.lock();
}

void VulkanMeshCommon::EndThreadLock() const
{
	m_Mutex.unlock();
}

void VulkanMeshCommon::AssertHasLock() const
{
	m_Mutex.assert_has_lock();
}
