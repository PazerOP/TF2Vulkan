#include "TF2Vulkan/IShaderAPI/IShaderAPI_StateManagerDynamic.h"
#include "interface/IMaterialInternal.h"
#include "interface/internal/IBufferPoolInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IStateManagerStatic.h"
#include "TF2Vulkan/VulkanFactories.h"
#include "VulkanMesh.h"

#include <TF2Vulkan/Util/FourCC.h>
#include "TF2Vulkan/Util/Misc.h"
#include <TF2Vulkan/Util/std_variant.h>

using namespace TF2Vulkan;

static std::atomic<uint32_t> s_VulkanGPUBufferIndex = 0;

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

void* VulkanGPUBuffer::GetBuffer(size_t size, bool truncate)
{
	AssertHasLock();
	LOG_FUNC_ANYTHREAD();

	if (truncate || m_CPUBuffer.size() < size)
		m_CPUBuffer.resize(size);

	return m_CPUBuffer.data();
}

void VulkanGPUBuffer::CommitModifications(size_t updateBegin, size_t updateSize)
{
	AssertHasLock();
	LOG_FUNC_ANYTHREAD();

	assert(updateSize > 0);

	if (Util::IsMainThread())
	{
		TF2VULKAN_PIX_MARKER("CommitModifications %s [%s]: %zu bytes @ offset %zu",
			vk::to_string(m_Usage).c_str(),
			IsDynamic() ? "dynamic" : "static",
			updateBegin, updateSize);
	}

	if (IsDynamic())
	{
		// Just memcpy into the dynamic buffer
		//if (updateSize > 0 || !std::holds_alternative<BufferPoolEntry>(m_Buffer))
			m_Buffer = g_ShaderDevice.GetBufferPool(m_Usage).Create(m_CPUBuffer.size(), m_CPUBuffer.data());
	}
	else
	{
		assert(std::holds_alternative<std::monostate>(m_Buffer) || std::holds_alternative<vma::AllocatedBuffer>(m_Buffer));

		auto entry = g_ShaderDevice
			.GetBufferPool(vk::BufferUsageFlagBits::eTransferSrc)
			.Create(updateSize, Util::OffsetPtr(m_CPUBuffer.data(), updateBegin));

		const auto& realPool = static_cast<const IBufferPoolInternal&>(entry.GetPool());
		const auto entryInfo = realPool.GetBufferInfo(entry);

		auto& cmdBuf = g_ShaderDevice.GetPrimaryCmdBuf();

		auto& dstBuf = Util::get_or_emplace<vma::AllocatedBuffer>(m_Buffer);
		if (!dstBuf || dstBuf.size() < m_CPUBuffer.size())
		{
			if (dstBuf)
				cmdBuf.AddResource(std::move(dstBuf));

			char dbgName[128];
			sprintf_s(dbgName, "VulkanGPUBuffer #%u [static] [%zu, %zu)", ++s_VulkanGPUBufferIndex,
				updateBegin, updateBegin + updateSize);

			dstBuf = Factories::BufferFactory{}
				.SetSize(m_CPUBuffer.size())
				.SetUsage(m_Usage | vk::BufferUsageFlagBits::eTransferDst)
				.SetMemoryType(vma::MemoryType::eGpuOnly)
				.SetDebugName(dbgName)
				.Create();
		}

		const auto region = vk::BufferCopy{}
			.setSize(updateSize)
			.setSrcOffset(entry.GetOffset())
			.setDstOffset(updateBegin);

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

	assert((firstIndex + indexCount) <= IndexCount());

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
		assert(!indexCount == !VertexCount());
		Warning(TF2VULKAN_PREFIX "No vertices\n");
		TF2VULKAN_PIX_MARKER("No vertices");
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

unsigned VulkanMesh::ComputeMemoryUsed()
{
	LOG_FUNC_ANYTHREAD();
	return Util::SafeConvert<unsigned>(m_IndexBuffer.ComputeMemoryUsed() + m_VertexBuffer.ComputeMemoryUsed());
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
}

void VulkanIndexBuffer::ModifyBegin(uint32_t firstIndex, uint32_t indexCount, IndexDesc_t& desc,
	bool read, bool write, bool replace)
{
	LOG_FUNC_ANYTHREAD();

	desc = {};
	desc.m_nIndexSize = sizeof(IndexFormatType) >> 1; // Why?
	desc.m_nFirstIndex = 0;
	desc.m_nOffset = 0;

	if (indexCount <= 0)
	{
		// Not thread safe at this point, but we don't want to unnecessarily
		// lock if indexCount is zero and we're just returning a pointer to
		// dummy data.
		assert(!m_ModifyIndexData);

		desc.m_pIndices = reinterpret_cast<IndexFormatType*>(&IShaderAPI_MeshManager::s_FallbackMeshData);
		return;
	}

	BeginThreadLock();
	assert(!m_ModifyIndexData); // Once more after the lock, just to be sure
	const auto indexDataSize = indexCount * IndexElementSize();
	const auto& modifyData = m_ModifyIndexData.emplace<ModifyData>({ firstIndex, indexCount });

	desc.m_pIndices = reinterpret_cast<IndexFormatType*>(Util::OffsetPtr(GetBuffer(indexDataSize, replace), firstIndex * IndexElementSize()));
	assert(desc.m_pIndices);
}

void VulkanIndexBuffer::ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	const bool shouldAllowRead = true;
	const bool shouldAllowWrite = !readOnly;
	return ModifyBegin(firstIndex, indexCount, desc, shouldAllowRead, shouldAllowWrite, false);
}

void VulkanIndexBuffer::ModifyEnd(IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	AssertCheckHeap();

	if (!m_ModifyIndexData)
		return;

	const auto& modify = m_ModifyIndexData.value();
	assert(IsDynamic() || modify.m_Count > 0);

	CommitModifications(modify.m_Offset * IndexElementSize(), modify.m_Count * IndexElementSize());

	ValidateData(m_IndexCount, 0, desc);

	m_ModifyIndexData.reset();

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
	ModifyBegin(append ? VertexCount() : 0, vertexCount, desc, false, true, true);
	return true;
}

bool VulkanIndexBuffer::Lock(int indexCount, bool append, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	assert(indexCount >= 0);
	ModifyBegin(append ? IndexCount() : 0, indexCount, desc, false, true, true);
	return true;
}

void VulkanVertexBuffer::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	if (!m_ModifyVertexData)
		return;

	auto& modify = m_ModifyVertexData.value();

	[[maybe_unused]] const auto oldModifySize = modify.m_Size;
	[[maybe_unused]] const auto oldVertexCount = m_VertexCount;

	assert(vertexCount >= 0);
	if (vertexCount > 0)
	{
		Util::SafeConvert(vertexCount * desc.m_ActualVertexSize, modify.m_Size);
		Util::SafeConvert(vertexCount, m_VertexCount);
	}

	ModifyEnd(desc);
}

void VulkanIndexBuffer::Unlock(int indexCount, IndexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	AssertCheckHeap();

	if (!m_ModifyIndexData)
		return;

	auto& modify = m_ModifyIndexData.value();

	[[maybe_unused]] const auto oldModifyCount = modify.m_Count;
	[[maybe_unused]] const auto oldIndexCount = m_IndexCount;

	assert(indexCount >= 0);
	if (indexCount > 0)
	{
		Util::SafeConvert(indexCount, modify.m_Count);
		Util::SafeConvert(modify.m_Count, m_IndexCount);
	}

	ModifyEnd(desc);
}

void VulkanVertexBuffer::ModifyEnd(VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	if (!m_ModifyVertexData)
		return;

	const auto& modify = m_ModifyVertexData.value();

	CommitModifications(modify.m_Offset, modify.m_Size);

	m_VertexCount = Util::algorithm::max(m_VertexCount, (modify.m_Size + modify.m_Offset) / desc.m_ActualVertexSize);

	ValidateData(modify.m_Size / desc.m_ActualVertexSize, desc);

	m_ModifyVertexData.reset();

	EndThreadLock();
}

void VulkanVertexBuffer::ModifyBegin(uint32_t firstVertex, uint32_t vertexCount, VertexDesc_t& desc,
	bool read, bool write, bool replace)
{
	LOG_FUNC_ANYTHREAD();

	desc = {};

	if (vertexCount <= 0)
	{
		// Not thread safe yet, but avoid unnecessary locks if just returning dummy data.
		assert(!m_ModifyVertexData);
		g_MeshManager.SetDummyDataPointers(desc);
		return;
	}

	BeginThreadLock();
	assert(!m_ModifyVertexData); // Once more, after the lock
	AssertCheckHeap();

	assert(!m_Format.IsUnknownFormat());

	VertexFormat::Element vtxElems[VERTEX_ELEMENT_NUMELEMENTS];
	size_t totalVtxSize;

	desc.m_NumBoneWeights = m_Format.m_BoneWeightCount;

	const auto vtxElemsCount = m_Format.GetVertexElements(vtxElems, std::size(vtxElems), &totalVtxSize);

	assert(!m_ModifyVertexData);
	const auto& modify = m_ModifyVertexData.emplace<ModifyData>({ firstVertex * totalVtxSize, vertexCount * totalVtxSize });
	assert(modify.m_Size > 0);

	g_MeshManager.ComputeVertexDescription(GetBuffer(modify.m_Offset + modify.m_Size, replace), m_Format, desc);

	//ValidateData(vertexCount, firstVertex, desc);
}

void VulkanVertexBuffer::Spew(int vertexCount, const VertexDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

template<typename T, size_t count, size_t alignment, typename = std::enable_if_t<std::is_floating_point_v<T>>>
static void ValidateType(const Shaders::vector<T, count, alignment>& v)
{
	for (size_t i = 0; i < count; i++)
	{
		if (!std::isfinite(v[i]))
			DebuggerBreakIfDebugging();
	}
}

template<typename T>
static void ValidateType(const void* base, int elementIndex, int elementSize)
{
	if (elementSize <= 0)
		return;

	const T& typed = *reinterpret_cast<const T*>(reinterpret_cast<const std::byte*>(base) + elementSize * elementIndex);
	ValidateType(typed);
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
