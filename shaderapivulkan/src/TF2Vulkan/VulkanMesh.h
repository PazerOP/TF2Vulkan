#pragma once

#include "VertexFormat.h"

#include <materialsystem/imesh.h>

#include <vector>

namespace Util
{
	union FourCC;
}

namespace TF2Vulkan
{
	class VulkanVertexBuffer final : public IVertexBuffer
	{
	public:
		explicit VulkanVertexBuffer(const VertexFormat& format);

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

	private:
		VertexFormat m_Format;

		template<typename T> void LockStream(VertexFormatFlags flag,
			std::vector<T>& vec, int& descCount, T*& descPtr, int count,
			const Util::FourCC& fallbackPtr);

		std::vector<float> m_Position;
		std::vector<float> m_BoneWeight;
		std::vector<unsigned char> m_BoneMatrixIndex;
		std::vector<float> m_Normal;
		std::vector<unsigned char> m_Color;
		std::vector<unsigned char> m_Specular;
		std::vector<float> m_TangentS;
		std::vector<float> m_TangentT;
		std::vector<float> m_Wrinkle;
		std::vector<float> m_UserData;
		std::vector<float> m_TexCoords[VERTEX_MAX_TEXTURE_COORDINATES];
	};

	class VulkanIndexBuffer final : public IIndexBuffer
	{
	public:
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

	private:
		std::vector<unsigned short> m_Indices;
	};

	class VulkanMesh final : public IMesh
	{
	public:
		explicit VulkanMesh(const VertexFormat& fmt);

		void SetPrimitiveType(MaterialPrimitiveType_t type) override;

		void Draw(int firstIndex, int indexCount) override;

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

		MaterialPrimitiveType_t m_PrimitiveType = MATERIAL_TRIANGLES;
	};
}
