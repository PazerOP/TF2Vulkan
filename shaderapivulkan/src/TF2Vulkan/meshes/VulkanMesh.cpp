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

int VulkanMesh::GetRoomRemaining() const
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();

	const auto vtxRoom = m_VertexBuffer->GetRoomRemaining();
	const auto idxRoom = m_IndexBuffer->GetRoomRemaining();

	assert(vtxRoom == idxRoom);
	return Util::algorithm::min(vtxRoom, idxRoom);
}

void VulkanMesh::OverrideVertexBuffer(IMesh* src)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();

	if (src)
	{
		if (!m_OriginalVertexBuffer)
			m_OriginalVertexBuffer = m_VertexBuffer;

		m_VertexBuffer = assert_cast<VulkanMesh*>(src)->m_VertexBuffer;
	}
	else
	{
		if (m_OriginalVertexBuffer)
			m_VertexBuffer = std::move(m_OriginalVertexBuffer);
	}
}

void VulkanMesh::OverrideIndexBuffer(IMesh* src)
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();

	if (src)
	{
		if (!m_OriginalIndexBuffer)
			m_OriginalIndexBuffer = m_IndexBuffer;

		m_IndexBuffer = assert_cast<VulkanMesh*>(src)->m_IndexBuffer;
	}
	else
	{
		if (m_OriginalIndexBuffer)
			m_IndexBuffer = std::move(m_OriginalIndexBuffer);
	}
}

void VulkanGPUBuffer::UpdateDynamicBuffer()
{
	auto lock = ScopeThreadLock();
	LOG_FUNC_ANYTHREAD();

	if (IsDynamic())
	{
		if (Util::IsMainThread())
		{
			TF2VULKAN_PIX_MARKER("UpdateDynamicBuffer %s [dynamic]: %zu bytes",
				vk::to_string(m_Usage).c_str(),
				m_CPUBuffer.size());
		}

		// Just memcpy into the dynamic buffer
		//if (updateSize > 0 || !std::holds_alternative<BufferPoolEntry>(m_Buffer))
		m_Buffer = g_ShaderDevice.GetBufferPool(m_Usage).Create(m_CPUBuffer.size(), m_CPUBuffer.data());
	}
}

void* VulkanGPUBuffer::GetBuffer(size_t size, bool truncate)
{
	AssertHasLock();
	LOG_FUNC_ANYTHREAD();

	if (Util::IsMainThread())
	{
		TF2VULKAN_PIX_MARKER("GetBuffer(size = %zu, truncate = %s) %s [%s]",
			m_CPUBuffer.size(), truncate ? "true" : "false",
			vk::to_string(m_Usage).c_str(),
			IsDynamic() ? "dynamic" : "static");
	}

	if (truncate || m_CPUBuffer.size() < size)
		m_CPUBuffer.resize(size);

	return m_CPUBuffer.data();
}

void VulkanGPUBuffer::CommitModifications(size_t updateBegin, size_t updateSize)
{
	AssertHasLock();
	LOG_FUNC_ANYTHREAD();

	if (!IsDynamic())
	{
		if (Util::IsMainThread())
		{
			TF2VULKAN_PIX_MARKER("CommitModifications %s [static]: %zu bytes @ offset %zu",
				vk::to_string(m_Usage).c_str(),
				updateBegin, updateSize);
		}

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
	m_VertexBuffer(std::make_shared<VulkanVertexBuffer>(fmt, isDynamic)),
	m_IndexBuffer(std::make_shared<VulkanIndexBuffer>(isDynamic))
{
}

VulkanMesh::VulkanMesh(const VertexFormat& fmt) :
	VulkanMesh(fmt, false)
{
}

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
	m_IndexBuffer->GetGPUBuffer(indexBuffer, indexBufferOffset);
	m_VertexBuffer->GetGPUBuffer(vertexBuffer, vertexBufferOffset);
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

	MarkAsDrawn();
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

void VulkanMesh::SetFlexMesh(IMesh* mesh, int vertexOffset)
{
	LOG_FUNC();
	if (mesh || vertexOffset)
		NOT_IMPLEMENTED_FUNC_NOBREAK(); // TODO: This goes into vPosFlex and vNormalFlex
}

VulkanIndexBuffer::VulkanIndexBuffer(bool isDynamic) :
	VulkanGPUBuffer(isDynamic, vk::BufferUsageFlagBits::eIndexBuffer)
{
}

void VulkanIndexBuffer::ModifyBegin(uint32_t firstIndex, uint32_t indexCount, IndexDesc_t& desc,
	bool read, bool write, bool truncate)
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

	desc.m_pIndices = reinterpret_cast<IndexFormatType*>(Util::OffsetPtr(GetBuffer(indexDataSize, truncate), firstIndex * IndexElementSize()));
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

void VulkanGPUBuffer::GetGPUBuffer(vk::Buffer& buffer, size_t& offset)
{
	if (IsDynamic())
		UpdateDynamicBuffer();

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

void VulkanVertexBuffer::Unlock(int vertexCount, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	if (!m_ModifyVertexData)
		return;

	auto& modify = m_ModifyVertexData.value();

	[[maybe_unused]] const auto oldModifySize = modify.m_Size;
	[[maybe_unused]] const auto oldVertexCount = m_VertexCount;

	Util::SafeConvert(vertexCount, m_VertexCount);

	modify.m_Size = m_VertexCount * Util::SafeConvert<uint32_t>(desc.m_ActualVertexSize);

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

	Util::SafeConvert(indexCount, m_IndexCount);

	Util::SafeConvert(m_IndexCount, modify.m_Count);

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
	bool read, bool write, bool truncate)
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

	g_MeshManager.ComputeVertexDescription(GetBuffer(modify.m_Offset + modify.m_Size, truncate), m_Format, desc);

	//ValidateData(vertexCount, firstVertex, desc);
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

VulkanDynamicMesh::VulkanDynamicMesh(const VertexFormat& fmt) :
	VulkanMesh(fmt, true)
{
}

void VulkanDynamicMesh::LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	if (m_HasDrawn)
	{
		m_FirstUndrawnIndex = 0;
		m_FirstUndrawnVertex = 0;
		m_TotalIndices = 0;
		m_TotalVertices = 0;
		m_HasDrawn = false;
	}

	return VulkanMesh::LockMesh(vertexCount, indexCount, desc);
}

void VulkanDynamicMesh::UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();
	m_TotalVertices += Util::SafeConvert<uint32_t>(vertexCount);
	m_TotalIndices += Util::SafeConvert<uint32_t>(indexCount);

	return BaseClass::UnlockMesh(vertexCount, indexCount, desc);
}

void VulkanDynamicMesh::Draw(int firstIndex, int indexCount)
{
	LOG_FUNC_ANYTHREAD();

	MarkAsDrawn();

	const bool isPointsOrInstQuads = GetPrimitiveType() == MATERIAL_POINTS || GetPrimitiveType() == MATERIAL_INSTANCED_QUADS;

	const bool ibOverride = HasIndexBufferOverride();
	const bool vbOverride = HasVertexBufferOverride();

	if (ibOverride || vbOverride || (m_TotalVertices > 0 && (m_TotalIndices > 0 || isPointsOrInstQuads)))
	{
		const uint32_t firstVertex = vbOverride ? 0 : m_FirstUndrawnVertex;
		//const uint32_t firstIndex = m_IndexOverride ? 0 : m_FirstUndrawnIndex;
		const uint32_t realIndexCount = ibOverride ? firstVertex : 0;
		const uint32_t baseIndex = ibOverride ? 0 : m_FirstUndrawnIndex;

		if (firstIndex == -1 && indexCount == 0)
		{
			// Draw the entire mesh
			if (ibOverride)
			{
				indexCount = IndexCount();
			}
			else
			{
				if (isPointsOrInstQuads)
					Util::SafeConvert(m_TotalVertices, indexCount);
				else
					Util::SafeConvert(m_TotalIndices, indexCount);
			}

			assert(indexCount > 0);
		}
		else
		{
			assert(firstIndex >= 0);
			firstIndex += baseIndex;
		}

		BaseClass::Draw(firstIndex, indexCount);
	}
	else
	{
		const auto idxCount = IndexCount();
		const auto vtxCount = VertexCount();
		if (idxCount > 0 && vtxCount > 0)
			Warning(TF2VULKAN_PREFIX "Skipping draw (%i indices and %i vertices)\n", idxCount, vtxCount);
	}
}

void VulkanDynamicMesh::MarkAsDrawn()
{
	LOG_FUNC_ANYTHREAD();
	m_HasDrawn = true;
}
