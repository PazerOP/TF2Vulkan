#pragma once

#include <TF2Vulkan/VertexFormat.h>
#include "TF2Vulkan/Util/MutexWrapper.h"

#include "interface/internal/IMeshInternal.h"

#include <atomic>
#include <variant>
#include <vector>

namespace Util
{
	union FourCC;
}

namespace TF2Vulkan
{
	class VulkanMeshCommon
	{
	private:
		using MutexType = Util::RecursiveMutexDbg;
	protected:
		std::unique_lock<MutexType> ScopeThreadLock() const { return std::unique_lock(m_Mutex); }
		void BeginThreadLock() const { m_Mutex.lock(); }
		void EndThreadLock() const { m_Mutex.unlock(); }
		void AssertHasLock() const { m_Mutex.assert_has_lock(); }

	private:
		mutable MutexType m_Mutex;
	};

	class VulkanGPUBuffer : virtual VulkanMeshCommon
	{
	public:
		VulkanGPUBuffer(bool isDynamic, vk::BufferUsageFlags usage);

		void GetGPUBuffer(vk::Buffer& buffer, size_t& offset);

		bool IsDynamic() const { return m_IsDynamic; }

	protected:
		void UpdateDynamicBuffer();
		void* GetBuffer(size_t size, bool truncate = false);
		void CommitModifications(size_t updateBegin, size_t updateSize);

		static void AssertCheckHeap();

	private:
		bool m_IsDynamic;

		vk::BufferUsageFlags m_Usage;
		std::variant<std::monostate, vma::AllocatedBuffer, BufferPoolEntry> m_Buffer;

		std::vector<std::byte> m_CPUBuffer;
	};

	class VulkanVertexBuffer final : public IVertexBufferInternal, public VulkanGPUBuffer, virtual VulkanMeshCommon
	{
	public:
		explicit VulkanVertexBuffer(const VertexFormat& format, bool isDynamic);

		int VertexCount() const override;
		VertexFormat_t GetVertexFormat() const override;

		bool IsDynamic() const override;

		void BeginCastBuffer(VertexFormat_t format) override { NOT_IMPLEMENTED_FUNC(); }
		void EndCastBuffer() override { NOT_IMPLEMENTED_FUNC(); }

		int GetRoomRemaining() const override { NOT_IMPLEMENTED_FUNC(); }

		bool Lock(int vertexCount, bool append, VertexDesc_t& desc) override;
		void Unlock(int vertexCount, VertexDesc_t& desc) override;
		void ModifyBegin(uint32_t firstVertex, uint32_t vertexCount, VertexDesc_t& desc, bool read, bool write, bool truncate);
		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, VertexDesc_t& desc);
		void ModifyEnd(VertexDesc_t& desc);

		void Spew(int vertexCount, const VertexDesc_t& desc) override { NOT_IMPLEMENTED_FUNC(); }

		void ValidateData(int vertexCount, const VertexDesc_t& desc) override;
		void ValidateData(uint32_t vertexCount, uint32_t firstVertex, const VertexDesc_t& desc);

		size_t ComputeMemoryUsed() const { NOT_IMPLEMENTED_FUNC(); }

	private:

		struct ModifyData
		{
			uint32_t m_Offset = 0;
			uint32_t m_Size = 0;
		};
		std::optional<ModifyData> m_ModifyVertexData;

		VertexFormat m_Format;
		uint32_t m_VertexCount = 0;
	};

	class VulkanIndexBuffer final : public IIndexBufferInternal, public VulkanGPUBuffer, virtual VulkanMeshCommon
	{
	public:
		VulkanIndexBuffer(bool isDynamic);

		static constexpr auto MATERIAL_INDEX_FORMAT = MATERIAL_INDEX_FORMAT_16BIT;
		using IndexFormatType = std::conditional_t<MATERIAL_INDEX_FORMAT == MATERIAL_INDEX_FORMAT_16BIT, uint16_t, uint32_t>;

		int IndexCount() const override;
		MaterialIndexFormat_t IndexFormat() const override;
		static constexpr size_t IndexElementSize() { return sizeof(IndexFormatType); }

		bool IsDynamic() const override;

		void BeginCastBuffer(MaterialIndexFormat_t format) override { NOT_IMPLEMENTED_FUNC(); }
		void EndCastBuffer() override { NOT_IMPLEMENTED_FUNC(); }

		int GetRoomRemaining() const override { NOT_IMPLEMENTED_FUNC(); }

		bool Lock(int maxIndexCount, bool append, IndexDesc_t& desc) override;
		void Unlock(int writtenIndexCount, IndexDesc_t& desc) override;

		void ModifyBegin(uint32_t firstIndex, uint32_t indexCount, IndexDesc_t& desc, bool read, bool write, bool truncate);
		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc) override;
		void ModifyEnd(IndexDesc_t& desc) override;

		void Spew(int indexCount, const IndexDesc_t& desc) override { NOT_IMPLEMENTED_FUNC(); }

		void ValidateData(int indexCount, const IndexDesc_t& desc) override;

		size_t ComputeMemoryUsed() const { NOT_IMPLEMENTED_FUNC(); }

	private:
		void ValidateData(uint32_t indexCount, uint32_t firstIndex, const IndexDesc_t& desc);

		size_t IndexDataSize() const { return IndexCount() * IndexElementSize(); }

		struct ModifyData
		{
			uint32_t m_Offset = 0;
			uint32_t m_Count = 0;
		};
		std::optional<ModifyData> m_ModifyIndexData;
		uint32_t m_IndexCount = 0;
	};

	class VulkanMesh : public IMeshInternal, protected virtual VulkanMeshCommon
	{
	protected:
		explicit VulkanMesh(const VertexFormat& fmt, bool isDynamic);

	public:
		explicit VulkanMesh(const VertexFormat& fmt);

		void SetPrimitiveType(MaterialPrimitiveType_t type) override final;
		MaterialPrimitiveType_t GetPrimitiveType() const { return m_PrimitiveType; }

		void Draw(CPrimList* lists, int listCount) override final;
		void Draw(int firstIndex, int indexCount) override;
		void DrawInternal(IVulkanCommandBuffer& cmdBuf, int firstIndex, int indexCount);

		void SetColorMesh(IMesh* colorMesh, int vertexOffset) override final;

		void CopyToMeshBuilder(int startVert, int vertCount,
			int startIndex, int indexCount, int indexOffset,
			CMeshBuilder& builder) override final;

		void Spew(int vertexCount, int indexCount, const MeshDesc_t& desc) override final;

		void ValidateData(int vertexCount, int indexCount, const MeshDesc_t& desc) override final;

		void LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;
		void UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;
		void ModifyBegin(int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc) override final;
		void ModifyEnd(MeshDesc_t& desc) override final;

		void ModifyBeginEx(bool readOnly, int firstVertex, int vertexCount, int firstIndex, int indexCount,
			MeshDesc_t& desc) override final;

		void SetFlexMesh(IMesh* mesh, int vertexOffset) override final;
		void DisableFlexMesh() override final;

		void MarkAsDrawn() override { LOG_FUNC_ANYTHREAD(); }
		unsigned ComputeMemoryUsed() override final;

		// IIndexBuffer
		int IndexCount() const override final;
		MaterialIndexFormat_t IndexFormat() const override final;
		void BeginCastBuffer(MaterialIndexFormat_t format) override final;
		bool Lock(int maxIndexCount, bool append, IndexDesc_t& desc) override final;
		void Unlock(int writtenIndexCount, IndexDesc_t& desc) override final;
		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc) override final;
		void ModifyEnd(IndexDesc_t& desc) override final;
		void Spew(int indexCount, const IndexDesc_t& desc) override final;
		void ValidateData(int indexCount, const IndexDesc_t& desc) override final;

		// IVertexBuffer
		int VertexCount() const override final;
		VertexFormat_t GetVertexFormat() const override final;
		void BeginCastBuffer(VertexFormat_t format) override final;
		bool Lock(int vertexCount, bool append, VertexDesc_t& desc) override final;
		void Unlock(int vertexCount, VertexDesc_t& desc) override final;
		void Spew(int vertexCount, const VertexDesc_t& desc) override final;
		void ValidateData(int vertexCount, const VertexDesc_t& desc) override final;
		void ModifyEnd(VertexDesc_t& desc);

		// Ambiguous methods
		bool IsDynamic() const override { LOG_FUNC_ANYTHREAD(); return false; }
		void EndCastBuffer() override final;
		int GetRoomRemaining() const override final;

		void OverrideVertexBuffer(IMesh* src);
		void OverrideIndexBuffer(IMesh* src);

		bool HasVertexBufferOverride() const { return !!m_OriginalVertexBuffer; }
		bool HasIndexBufferOverride() const { return !!m_OriginalIndexBuffer; }

	private:
		std::shared_ptr<VulkanVertexBuffer> m_VertexBuffer;
		std::shared_ptr<VulkanIndexBuffer> m_IndexBuffer;

		std::shared_ptr<VulkanVertexBuffer> m_OriginalVertexBuffer;
		std::shared_ptr<VulkanIndexBuffer> m_OriginalIndexBuffer;

		IMesh* m_ColorMesh = nullptr;
		int m_ColorMeshVertexOffset = 0;

		MaterialPrimitiveType_t m_PrimitiveType = MATERIAL_TRIANGLES;
	};

	class VulkanDynamicMesh final : public VulkanMesh
	{
		using BaseClass = VulkanMesh;
	public:
		VulkanDynamicMesh(const VertexFormat& fmt);

		bool IsDynamic() const override final { LOG_FUNC_ANYTHREAD(); return true; }

		void LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;
		void UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;

		void Draw(int firstIndex, int indexCount) override;

		void MarkAsDrawn() override;

	private:
		uint32_t m_FirstUndrawnIndex = 0;
		uint32_t m_FirstUndrawnVertex = 0;

		uint32_t m_TotalVertices = 0;
		uint32_t m_TotalIndices = 0;

		bool m_HasDrawn = false;
	};

	inline int VulkanMesh::IndexCount() const
	{
		LOG_FUNC_ANYTHREAD();
		return m_IndexBuffer->IndexCount();
	}

	inline MaterialIndexFormat_t VulkanMesh::IndexFormat() const
	{
		LOG_FUNC_ANYTHREAD();
		return m_IndexBuffer->IndexFormat();
	}

	inline void VulkanMesh::BeginCastBuffer(MaterialIndexFormat_t format)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasIndexBufferOverride());
		return m_IndexBuffer->BeginCastBuffer(format);
	}

	inline bool VulkanMesh::Lock(int maxIndexCount, bool append, IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasIndexBufferOverride());
		return m_IndexBuffer->Lock(maxIndexCount, append, desc);
	}

	inline void VulkanMesh::Unlock(int writtenIndexCount, IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasIndexBufferOverride());
		return m_IndexBuffer->Unlock(writtenIndexCount, desc);
	}

	inline void VulkanMesh::ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasIndexBufferOverride());
		return m_IndexBuffer->ModifyBegin(readOnly, firstIndex, indexCount, desc);
	}

	inline void VulkanMesh::ModifyEnd(IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasIndexBufferOverride());
		m_IndexBuffer->ModifyEnd(desc);
	}

	inline void VulkanMesh::Spew(int indexCount, const IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return m_IndexBuffer->Spew(indexCount, desc);
	}

	inline void VulkanMesh::ValidateData(int indexCount, const IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return m_IndexBuffer->ValidateData(indexCount, desc);
	}

	inline int VulkanMesh::VertexCount() const
	{
		LOG_FUNC_ANYTHREAD();
		return m_VertexBuffer->VertexCount();
	}

	inline VertexFormat_t VulkanMesh::GetVertexFormat() const
	{
		LOG_FUNC_ANYTHREAD();
		return m_VertexBuffer->GetVertexFormat();
	}

	inline void VulkanMesh::BeginCastBuffer(VertexFormat_t format)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasVertexBufferOverride());
		return m_VertexBuffer->BeginCastBuffer(format);
	}

	inline bool VulkanMesh::Lock(int vertexCount, bool append, VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasVertexBufferOverride());
		return m_VertexBuffer->Lock(vertexCount, append, desc);
	}

	inline void VulkanMesh::Unlock(int vertexCount, VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		//assert(!HasVertexBufferOverride());
		return m_VertexBuffer->Unlock(vertexCount, desc);
	}

	inline void VulkanMesh::Spew(int vertexCount, const VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return m_VertexBuffer->Spew(vertexCount, desc);
	}

	inline int VulkanIndexBuffer::IndexCount() const
	{
		LOG_FUNC_ANYTHREAD();
		return Util::SafeConvert<int>(m_IndexCount);
	}

	inline int VulkanVertexBuffer::VertexCount() const
	{
		LOG_FUNC_ANYTHREAD();
		return Util::SafeConvert<int>(m_VertexCount);
	}

	inline MaterialIndexFormat_t VulkanIndexBuffer::IndexFormat() const
	{
		LOG_FUNC_ANYTHREAD();
		return MATERIAL_INDEX_FORMAT;
	}

	inline bool VulkanVertexBuffer::IsDynamic() const
	{
		LOG_FUNC_ANYTHREAD();
		return VulkanGPUBuffer::IsDynamic();
	}

	inline bool VulkanIndexBuffer::IsDynamic() const
	{
		LOG_FUNC_ANYTHREAD();
		return VulkanGPUBuffer::IsDynamic();
	}

	inline VertexFormat_t VulkanVertexBuffer::GetVertexFormat() const
	{
		LOG_FUNC_ANYTHREAD();
		return m_Format;
	}

	inline void VulkanVertexBuffer::ModifyBegin(bool readOnly, int firstVertex, int vertexCount, VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return ModifyBegin(firstVertex, vertexCount, desc, true, !readOnly, false);
	}

	inline void VulkanMesh::ModifyBegin(int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return ModifyBeginEx(false, firstVertex, vertexCount, firstIndex, indexCount, desc);
	}

	inline void VulkanVertexBuffer::ValidateData(int vertexCount, const VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return ValidateData(Util::SafeConvert<uint32_t>(vertexCount), 0, desc);
	}

	inline void VulkanMesh::ModifyEnd(MeshDesc_t& desc)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		ModifyEnd(static_cast<VertexDesc_t&>(desc));
		ModifyEnd(static_cast<IndexDesc_t&>(desc));
	}

	inline void VulkanMesh::ModifyEnd(VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		m_VertexBuffer->ModifyEnd(desc);
	}

	inline void VulkanMesh::UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();

		//if (vertexCount || !HasVertexBufferOverride())
			Unlock(vertexCount, static_cast<VertexDesc_t&>(desc));
		//if (indexCount || !HasIndexBufferOverride())
			Unlock(indexCount, static_cast<IndexDesc_t&>(desc));
	}

	inline void VulkanMesh::LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		const auto result1 = Lock(vertexCount, false, static_cast<VertexDesc_t&>(desc));
		const auto result2 = Lock(indexCount, false, static_cast<IndexDesc_t&>(desc));
		assert(result1 && result2);
	}

	inline void VulkanMesh::ValidateData(int vertexCount, int indexCount, const MeshDesc_t& desc)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		ValidateData(vertexCount, static_cast<const VertexDesc_t&>(desc));
		ValidateData(indexCount, static_cast<const IndexDesc_t&>(desc));
	}

	inline void VulkanMesh::DisableFlexMesh()
	{
		LOG_FUNC_ANYTHREAD();
		SetFlexMesh(nullptr, 0);
	}

	inline void VulkanMesh::Spew(int vertexCount, int indexCount, const MeshDesc_t& desc)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		Spew(vertexCount, static_cast<const VertexDesc_t&>(desc));
		Spew(vertexCount, static_cast<const IndexDesc_t&>(desc));
	}

	inline unsigned VulkanMesh::ComputeMemoryUsed()
	{
		LOG_FUNC_ANYTHREAD();
		return Util::SafeConvert<unsigned>(m_IndexBuffer->ComputeMemoryUsed() + m_VertexBuffer->ComputeMemoryUsed());
	}

	inline void VulkanMesh::ValidateData(int vertexCount, const VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		return m_VertexBuffer->ValidateData(vertexCount, desc);
	}

	inline bool VulkanVertexBuffer::Lock(int vertexCount, bool append, VertexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		assert(vertexCount >= 0);
		ModifyBegin(append ? VertexCount() : 0, vertexCount, desc, false, true, true);
		return true;
	}

	inline bool VulkanIndexBuffer::Lock(int indexCount, bool append, IndexDesc_t& desc)
	{
		LOG_FUNC_ANYTHREAD();
		assert(indexCount >= 0);
		ModifyBegin(append ? IndexCount() : 0, indexCount, desc, false, true, true);
		return true;
	}

	inline void VulkanMesh::ModifyBeginEx(bool readOnly, int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		m_VertexBuffer->ModifyBegin(readOnly, firstVertex, vertexCount, desc);
		ModifyBegin(readOnly, firstIndex, indexCount, static_cast<IndexDesc_t&>(desc));
	}

	inline void VulkanMesh::EndCastBuffer()
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		m_VertexBuffer->EndCastBuffer();
		m_IndexBuffer->EndCastBuffer();
	}

	inline void VulkanMesh::SetPrimitiveType(MaterialPrimitiveType_t type)
	{
		auto lock = ScopeThreadLock();
		LOG_FUNC_ANYTHREAD();
		m_PrimitiveType = type;
	}
}
