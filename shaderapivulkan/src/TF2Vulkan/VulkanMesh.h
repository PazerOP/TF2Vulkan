#pragma once

#include <TF2Vulkan/VertexFormat.h>

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
	class VulkanGPUBuffer
	{
	public:
		explicit VulkanGPUBuffer(bool isDynamic);

		void GetGPUBuffer(vk::Buffer& buffer, size_t& offset) const;

		bool IsDynamic() const { return m_IsDynamic; }

	protected:
		void UpdateInnerBuffer(const char* dbgName, const void* newData, size_t newSize, vk::BufferUsageFlags usage);

	private:
		bool m_IsDynamic;

		std::variant<std::monostate, vma::AllocatedBuffer, BufferPoolEntry> m_Buffer;
	};

	class VulkanVertexBuffer final : public IVertexBufferInternal, public VulkanGPUBuffer
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

		void Spew(int vertexCount, const VertexDesc_t& desc) override;

		void ValidateData(int vertexCount, const VertexDesc_t& desc) override;

		const std::byte* VertexData() const;
		size_t VertexDataSize() const;

	private:
		VertexFormat m_Format;

		std::vector<std::byte> m_DataBuffer;
		size_t m_VertexCount = 0;
	};

	class VulkanIndexBuffer final : public IIndexBufferInternal, public VulkanGPUBuffer
	{
	public:
		VulkanIndexBuffer(bool isDynamic);

		int IndexCount() const override;
		MaterialIndexFormat_t IndexFormat() const override;

		bool IsDynamic() const override;

		void BeginCastBuffer(MaterialIndexFormat_t format) override;
		void EndCastBuffer() override;

		int GetRoomRemaining() const override;

		bool Lock(int maxIndexCount, bool append, IndexDesc_t& desc) override;
		void Unlock(int writtenIndexCount, IndexDesc_t& desc) override;

		void ModifyBegin(bool readOnly, int firstIndex, int indexCount, IndexDesc_t& desc) override;
		void ModifyEnd(IndexDesc_t& desc) override;

		void Spew(int indexCount, const IndexDesc_t& desc) override;

		void ValidateData(int indexCount, const IndexDesc_t& desc) override;

		const unsigned short* IndexData() const;
		size_t IndexDataSize() const;

	private:
		std::vector<unsigned short> m_Indices;
	};

	class VulkanMesh final : public IMeshInternal
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
		void ModifyBegin(int firstVertex, int vertexCount, int firstIndex, int indexCount, MeshDesc_t& desc) override;
		void ModifyEnd(MeshDesc_t& desc) override;
		void UnlockMesh(int vertexCount, int indexCount, MeshDesc_t& desc) override;

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
