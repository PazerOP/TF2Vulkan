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
		std::unique_lock<MutexType> ScopeThreadLock() const;
		void BeginThreadLock() const;
		void EndThreadLock() const;
		void AssertHasLock() const;

	private:
		mutable MutexType m_Mutex;
	};

	class VulkanGPUBuffer : virtual VulkanMeshCommon
	{
	public:
		VulkanGPUBuffer(bool isDynamic, vk::BufferUsageFlags usage);

		void GetGPUBuffer(vk::Buffer& buffer, size_t& offset) const;

		bool IsDynamic() const { return m_IsDynamic; }

	protected:
		struct ModifyData
		{
			constexpr ModifyData() = default;

			void* m_Data = nullptr;
			size_t m_DataLength = size_t(-1);
			size_t m_DataOffset = size_t(-1);

		private:
			BufferPoolEntry m_PoolEntry;
			friend class VulkanGPUBuffer;
		};

		[[nodiscard]] ModifyData GetBufferData(size_t size, size_t offset, bool read, bool write);
		void CommitModifications(const ModifyData& modification);

		static void AssertCheckHeap();

	private:
		bool m_IsDynamic;

		vk::BufferUsageFlags m_Usage;
		std::variant<std::monostate, vma::AllocatedBuffer, BufferPoolEntry> m_Buffer;
	};

	class VulkanVertexBuffer final : public IVertexBufferInternal, public VulkanGPUBuffer, virtual VulkanMeshCommon
	{
	public:
		explicit VulkanVertexBuffer(const VertexFormat& format, bool isDynamic);

		int VertexCount() const override;
		VertexFormat_t GetVertexFormat() const override;

		bool IsDynamic() const override;

		void BeginCastBuffer(VertexFormat_t format) override;
		void EndCastBuffer() override;

		int GetRoomRemaining() const override;

		bool Lock(int vertexCount, bool append, VertexDesc_t& desc) override;
		void Unlock(int vertexCount, VertexDesc_t& desc) override;
		void ModifyBegin(int firstVertex, int vertexCount, VertexDesc_t& desc, bool read, bool write);
		void ModifyBegin(uint32_t firstVertex, uint32_t vertexCount, VertexDesc_t& desc, bool read, bool write);
		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, VertexDesc_t& desc);
		void ModifyEnd(VertexDesc_t& desc);

		void Spew(int vertexCount, const VertexDesc_t& desc) override;

		void ValidateData(int vertexCount, const VertexDesc_t& desc) override;

	private:
		void ValidateData(uint32_t vertexCount, uint32_t firstVertex, const VertexDesc_t& desc);

		struct ModifyData : VulkanGPUBuffer::ModifyData
		{
			constexpr ModifyData() = default;
			ModifyData(VulkanGPUBuffer::ModifyData&& base) : VulkanGPUBuffer::ModifyData(std::move(base)) {}
			uint32_t m_VertexCount = 0;
		};
		std::optional<ModifyData> m_ModifyData;

		VertexFormat m_Format;

		uint32_t m_TotalVertexCount = 0;
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

		void BeginCastBuffer(MaterialIndexFormat_t format) override;
		void EndCastBuffer() override;

		int GetRoomRemaining() const override;

		bool Lock(int maxIndexCount, bool append, IndexDesc_t& desc) override;
		void Unlock(int writtenIndexCount, IndexDesc_t& desc) override;

		void ModifyBegin(int firstIndex, int indexCount, IndexDesc_t& desc, bool read, bool write);
		void ModifyBegin(uint32_t firstIndex, uint32_t indexCount, IndexDesc_t& desc, bool read, bool write);
		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc) override;
		void ModifyEnd(IndexDesc_t& desc) override;

		void Spew(int indexCount, const IndexDesc_t& desc) override;

		void ValidateData(int indexCount, const IndexDesc_t& desc) override;

	private:
		void ValidateData(uint32_t indexCount, uint32_t firstIndex, const IndexDesc_t& desc);

		struct ModifyData : VulkanGPUBuffer::ModifyData
		{
			constexpr ModifyData() = default;
			ModifyData(VulkanGPUBuffer::ModifyData&& base) : VulkanGPUBuffer::ModifyData(std::move(base)) {}
		};
		std::optional<ModifyData> m_ModifyData;

		uint32_t m_TotalIndexCount = 0;
	};

	class VulkanMesh final : public IMeshInternal, virtual VulkanMeshCommon
	{
	public:
		explicit VulkanMesh(const VertexFormat& fmt, bool isDynamic);

		void SetPrimitiveType(MaterialPrimitiveType_t type) override;

		void Draw(int firstIndex, int indexCount) override;
		void DrawInternal(IVulkanCommandBuffer& cmdBuf, int firstIndex, int indexCount);

		void SetColorMesh(IMesh* colorMesh, int vertexOffset) override;

		void Draw(CPrimList* lists, int listCount) override;

		void CopyToMeshBuilder(int startVert, int vertCount,
			int startIndex, int indexCount, int indexOffset,
			CMeshBuilder& builder) override;

		void Spew(int vertexCount, int indexCount, const MeshDesc_t& desc) override;

		void ValidateData(int vertexCount, int indexCount, const MeshDesc_t& desc) override;

		void LockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;
		void UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;
		void ModifyBegin(int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc) override;
		void ModifyEnd(MeshDesc_t& desc) override;

		void ModifyBeginEx(bool readOnly, int firstVertex, int vertexCount, int firstIndex, int indexCount,
			MeshDesc_t& desc) override;

		void SetFlexMesh(IMesh* mesh, int vertexOffset) override;
		void DisableFlexMesh() override;

		void MarkAsDrawn() override;
		unsigned ComputeMemoryUsed() override;

		// IIndexBuffer
		int IndexCount() const override;
		MaterialIndexFormat_t IndexFormat() const override;
		void BeginCastBuffer(MaterialIndexFormat_t format) override;
		bool Lock(int maxIndexCount, bool append, IndexDesc_t& desc) override;
		void Unlock(int writtenIndexCount, IndexDesc_t& desc) override;
		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc) override;
		void ModifyEnd(IndexDesc_t& desc) override;
		void Spew(int indexCount, const IndexDesc_t& desc) override;
		void ValidateData(int indexCount, const IndexDesc_t& desc) override;

		// IVertexBuffer
		int VertexCount() const override;
		VertexFormat_t GetVertexFormat() const override;
		void BeginCastBuffer(VertexFormat_t format) override;
		bool Lock(int vertexCount, bool append, VertexDesc_t& desc) override;
		void Unlock(int vertexCount, VertexDesc_t& desc) override;
		void Spew(int vertexCount, const VertexDesc_t& desc) override;
		void ValidateData(int vertexCount, const VertexDesc_t& desc) override;

		// Ambiguous methods
		bool IsDynamic() const override;
		void EndCastBuffer() override;
		int GetRoomRemaining() const override;

		VulkanVertexBuffer m_VertexBuffer;
		VulkanIndexBuffer m_IndexBuffer;

		IMesh* m_ColorMesh = nullptr;
		int m_ColorMeshVertexOffset = 0;

		MaterialPrimitiveType_t m_PrimitiveType = MATERIAL_TRIANGLES;
	};
}
