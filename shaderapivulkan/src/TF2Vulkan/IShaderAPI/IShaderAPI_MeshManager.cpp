#include "IShaderAPI_MeshManager.h"

#include <materialsystem/imesh.h>

using namespace TF2Vulkan;

IShaderAPI_MeshManager::IShaderAPI_MeshManager() :
	m_FlexMesh(VertexFormat(VertexFormatFlags(VertexFormatFlags::Position | VertexFormatFlags::Normal | VertexFormatFlags::Wrinkle)), true)
{

}

int IShaderAPI_MeshManager::GetMaxVerticesToRender(IMaterial* material)
{
	LOG_FUNC();

	VertexFormat fmt(material->GetVertexFormat());
	fmt.RemoveFlags(VertexFormatFlags::Meta_Compressed);

	const auto vtxSize = fmt.GetVertexSize();
	const auto currentDynamicVBSize = GetCurrentDynamicVBSize();

	if (vtxSize < 1)
	{
		Warning(TF2VULKAN_PREFIX "Invalid vertex size %i for material %s (%s)\n",
			vtxSize, material->GetName(), material->GetShaderName());
		return currentDynamicVBSize;
	}

	return currentDynamicVBSize / vtxSize;
}

int IShaderAPI_MeshManager::GetMaxIndicesToRender()
{
	LOG_FUNC();

	// Technically we're "unlimited" (constrained by (v)ram only)
	return INDEX_BUFFER_SIZE;
}

IMesh* IShaderAPI_MeshManager::GetFlexMesh()
{
	return &m_FlexMesh;
}

int IShaderAPI_MeshManager::GetCurrentDynamicVBSize()
{
	LOG_FUNC();
	// Technically we're "unlimited" (constrained by (v)ram only)
	return DYNAMIC_VERTEX_BUFFER_MEMORY;
}

IMesh* IShaderAPI_MeshManager::GetDynamicMesh(IMaterial* material, int hwSkinBoneCount,
	bool buffered, IMesh* vertexOverride, IMesh* indexOverride)
{
	LOG_FUNC();
	return GetDynamicMeshEx(material, material->GetVertexFormat(), hwSkinBoneCount, buffered, vertexOverride, indexOverride);
}

IMesh* IShaderAPI_MeshManager::GetDynamicMeshEx(IMaterial* material, VertexFormat_t vertexFormat,
	int hwSkinBoneCount, bool buffered, IMesh* vertexOverride, IMesh* indexOverride)
{
	LOG_FUNC();

	const VertexFormat fmt(vertexFormat);
	assert(hwSkinBoneCount == 0);
	assert(fmt.m_BoneWeightCount == 0);
	assert(!(fmt.m_Flags & VertexFormatFlags::BoneIndex));

	return &m_DynamicMeshes.try_emplace(fmt, fmt, true).first->second;
}

CMeshBuilder* IShaderAPI_MeshManager::GetVertexModifyBuilder()
{
	LOG_FUNC();
	return &m_MeshBuilder;
}
