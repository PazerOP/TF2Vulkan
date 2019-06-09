#pragma once

#include "IShaderAPI_TextureManager.h"
#include "TF2Vulkan/VertexFormat.h"
#include "TF2Vulkan/meshes/VulkanMesh.h"

namespace TF2Vulkan
{
	class IShaderAPI_MeshManager : public IShaderAPI_TextureManager
	{
	public:
		IShaderAPI_MeshManager();

		void GetMaxToRender(IMesh* mesh, bool maxUntilFlush, int* maxVerts, int* maxIndices) override final { NOT_IMPLEMENTED_FUNC(); }

		int GetMaxVerticesToRender(IMaterial* material) override final;
		int GetMaxIndicesToRender() override final;

		IMesh* GetFlexMesh() override final;

		int GetCurrentDynamicVBSize() override final;
		void DestroyVertexBuffers(bool exitingLevel) override final { NOT_IMPLEMENTED_FUNC_NOBREAK(); }

		CMeshBuilder* GetVertexModifyBuilder() override final;
		IMesh* GetDynamicMesh(IMaterial* material, int hwSkinBoneCount, bool buffered,
			IMesh* vertexOverride, IMesh* indexOverride) override final;
		IMesh* GetDynamicMeshEx(IMaterial* material, VertexFormat_t vertexFormat, int hwSkinBoneCount,
			bool buffered, IMesh* vertexOverride, IMesh* indexOverride) override final;

		const ActiveMeshData& GetActiveMesh() override final { return m_ActiveMesh.top(); }
		void PushActiveMesh(const ActiveMeshData& mesh) override final { m_ActiveMesh.push(mesh); }
		void PopActiveMesh() override final { m_ActiveMesh.pop(); }

		static std::aligned_storage_t<256> s_FallbackMeshData;
		static void ComputeVertexDescription(void* buffer, VertexFormat fmt, VertexDesc_t& desc);

	private:
		[[deprecated]] void ComputeVertexDescription(unsigned char* buffer, VertexFormat_t fmt, MeshDesc_t& desc) const override final;

		std::unordered_map<VertexFormat, VulkanMesh> m_DynamicMeshes;
		VulkanMesh m_FlexMesh;

		CMeshBuilder m_MeshBuilder;

		std::stack<ActiveMeshData, std::vector<ActiveMeshData>> m_ActiveMesh;
	};

	extern IShaderAPI_MeshManager& g_MeshManager;
}
