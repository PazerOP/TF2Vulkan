#pragma once

#include "IShaderAPI_TextureManager.h"
#include "TF2Vulkan/VertexFormat.h"
#include "TF2Vulkan/VulkanMesh.h"

namespace TF2Vulkan
{
	class IShaderAPI_MeshManager : public IShaderAPI_TextureManager
	{
	public:
		void GetMaxToRender(IMesh* mesh, bool maxUntilFlush, int* maxVerts, int* maxIndices) override final { NOT_IMPLEMENTED_FUNC(); }

		int GetMaxVerticesToRender(IMaterial* material) override final;
		int GetMaxIndicesToRender() override final;

		IMesh* GetFlexMesh() override final { NOT_IMPLEMENTED_FUNC(); }

		int GetCurrentDynamicVBSize() override final;
		void DestroyVertexBuffers(bool exitingLevel) override final { NOT_IMPLEMENTED_FUNC(); }

		CMeshBuilder* GetVertexModifyBuilder() override final;
		IMesh* GetDynamicMesh(IMaterial* material, int hwSkinBoneCount, bool buffered,
			IMesh* vertexOverride, IMesh* indexOverride) override final;
		IMesh* GetDynamicMeshEx(IMaterial* material, VertexFormat_t vertexFormat, int hwSkinBoneCount,
			bool buffered, IMesh* vertexOverride, IMesh* indexOverride) override final;

		const ActiveMeshData& GetActiveMesh() override final { return m_ActiveMesh.top(); }
		void PushActiveMesh(const ActiveMeshData& mesh) override final { m_ActiveMesh.push(mesh); }
		void PopActiveMesh() override final { m_ActiveMesh.pop(); }

	private:
		std::unordered_map<VertexFormat, VulkanMesh> m_DynamicMeshes;

		CMeshBuilder m_MeshBuilder;

		std::stack<ActiveMeshData, std::vector<ActiveMeshData>> m_ActiveMesh;
	};
}
