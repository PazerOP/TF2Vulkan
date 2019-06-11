#include "IShaderAPI_MeshManager.h"

#include <materialsystem/imesh.h>

using namespace TF2Vulkan;

std::aligned_storage_t<256> IShaderAPI_MeshManager::s_FallbackMeshData;

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

	VertexFormat fmt(vertexFormat);
	fmt.SetBoneWeightCount(hwSkinBoneCount);

	return &m_DynamicMeshes.try_emplace(fmt, fmt, true).first->second;
}

void IShaderAPI_MeshManager::SetDummyDataPointers(VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	desc.m_pPosition = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pBoneWeight = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pBoneMatrixIndex = reinterpret_cast<unsigned char*>(&s_FallbackMeshData);
	desc.m_pNormal = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pColor = reinterpret_cast<unsigned char*>(&s_FallbackMeshData);
	desc.m_pSpecular = reinterpret_cast<unsigned char*>(&s_FallbackMeshData);
	desc.m_pTangentS = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pTangentT = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pWrinkle = reinterpret_cast<float*>(&s_FallbackMeshData);
	desc.m_pUserData = reinterpret_cast<float*>(&s_FallbackMeshData);
	for (auto& tc : desc.m_pTexCoord)
		tc = reinterpret_cast<float*>(&s_FallbackMeshData);
}

void IShaderAPI_MeshManager::ComputeVertexDescription(void* buffer, VertexFormat fmt, VertexDesc_t& desc)
{
	LOG_FUNC_ANYTHREAD();

	// Set default dummy pointers
	SetDummyDataPointers(desc);

	VertexFormat::Element vtxElems[VERTEX_ELEMENT_NUMELEMENTS];
	size_t totalVtxSize;

	desc.m_NumBoneWeights = fmt.m_BoneWeightCount;

	const auto vtxElemsCount = fmt.GetVertexElements(vtxElems, std::size(vtxElems), &totalVtxSize);
	Util::SafeConvert(totalVtxSize, desc.m_ActualVertexSize);
	desc.m_CompressionType = fmt.GetCompressionType();

	for (uint_fast8_t i = 0; i < vtxElemsCount; i++)
	{
		const auto& vtxElem = vtxElems[i];

		const auto UpdateElementParams = [&](auto& sizeParam, auto*& dataPtrParam)
		{
			using ptrType = std::decay_t<decltype(dataPtrParam)>;
			Util::SafeConvert(totalVtxSize, sizeParam);

			if (buffer)
				dataPtrParam = reinterpret_cast<ptrType>(reinterpret_cast<std::byte*>(buffer) + vtxElem.m_Offset);
		};

		switch (vtxElem.m_Type->m_Element)
		{
		default:
			assert(!"Unknown vertex element type");
		case VERTEX_ELEMENT_NONE:
			break;

		case VERTEX_ELEMENT_TEXCOORD1D_0:
		case VERTEX_ELEMENT_TEXCOORD1D_1:
		case VERTEX_ELEMENT_TEXCOORD1D_2:
		case VERTEX_ELEMENT_TEXCOORD1D_3:
		case VERTEX_ELEMENT_TEXCOORD1D_4:
		case VERTEX_ELEMENT_TEXCOORD1D_5:
		case VERTEX_ELEMENT_TEXCOORD1D_6:
		case VERTEX_ELEMENT_TEXCOORD1D_7:
		case VERTEX_ELEMENT_TEXCOORD2D_0:
		case VERTEX_ELEMENT_TEXCOORD2D_1:
		case VERTEX_ELEMENT_TEXCOORD2D_2:
		case VERTEX_ELEMENT_TEXCOORD2D_3:
		case VERTEX_ELEMENT_TEXCOORD2D_4:
		case VERTEX_ELEMENT_TEXCOORD2D_5:
		case VERTEX_ELEMENT_TEXCOORD2D_6:
		case VERTEX_ELEMENT_TEXCOORD2D_7:
		case VERTEX_ELEMENT_TEXCOORD3D_0:
		case VERTEX_ELEMENT_TEXCOORD3D_1:
		case VERTEX_ELEMENT_TEXCOORD3D_2:
		case VERTEX_ELEMENT_TEXCOORD3D_3:
		case VERTEX_ELEMENT_TEXCOORD3D_4:
		case VERTEX_ELEMENT_TEXCOORD3D_5:
		case VERTEX_ELEMENT_TEXCOORD3D_6:
		case VERTEX_ELEMENT_TEXCOORD3D_7:
		case VERTEX_ELEMENT_TEXCOORD4D_0:
		case VERTEX_ELEMENT_TEXCOORD4D_1:
		case VERTEX_ELEMENT_TEXCOORD4D_2:
		case VERTEX_ELEMENT_TEXCOORD4D_3:
		case VERTEX_ELEMENT_TEXCOORD4D_4:
		case VERTEX_ELEMENT_TEXCOORD4D_5:
		case VERTEX_ELEMENT_TEXCOORD4D_6:
		case VERTEX_ELEMENT_TEXCOORD4D_7:
		{
			const auto texCoordIdx = (vtxElem.m_Type->m_Element - VERTEX_ELEMENT_TEXCOORD1D_0) % VERTEX_MAX_TEXTURE_COORDINATES;
			UpdateElementParams(desc.m_VertexSize_TexCoord[texCoordIdx], desc.m_pTexCoord[texCoordIdx]);
			break;
		}

		case VERTEX_ELEMENT_BONEINDEX:
			UpdateElementParams(desc.m_VertexSize_BoneMatrixIndex, desc.m_pBoneMatrixIndex);
			break;

		case VERTEX_ELEMENT_BONEWEIGHTS1:
		case VERTEX_ELEMENT_BONEWEIGHTS2:
		case VERTEX_ELEMENT_BONEWEIGHTS3:
		case VERTEX_ELEMENT_BONEWEIGHTS4:
			UpdateElementParams(desc.m_VertexSize_BoneWeight, desc.m_pBoneWeight);
			break;

		case VERTEX_ELEMENT_USERDATA1:
		case VERTEX_ELEMENT_USERDATA2:
		case VERTEX_ELEMENT_USERDATA3:
		case VERTEX_ELEMENT_USERDATA4:
			UpdateElementParams(desc.m_VertexSize_UserData, desc.m_pUserData);
			break;

		case VERTEX_ELEMENT_POSITION:
			UpdateElementParams(desc.m_VertexSize_Position, desc.m_pPosition);
			break;
		case VERTEX_ELEMENT_NORMAL:
			UpdateElementParams(desc.m_VertexSize_Normal, desc.m_pNormal);
			break;
		case VERTEX_ELEMENT_COLOR:
			UpdateElementParams(desc.m_VertexSize_Color, desc.m_pColor);
			break;
		case VERTEX_ELEMENT_SPECULAR:
			UpdateElementParams(desc.m_VertexSize_Specular, desc.m_pSpecular);
			break;
		case VERTEX_ELEMENT_TANGENT_S:
			UpdateElementParams(desc.m_VertexSize_TangentS, desc.m_pTangentS);
			break;
		case VERTEX_ELEMENT_TANGENT_T:
			UpdateElementParams(desc.m_VertexSize_TangentT, desc.m_pTangentT);
			break;
		case VERTEX_ELEMENT_WRINKLE:
			UpdateElementParams(desc.m_VertexSize_Wrinkle, desc.m_pWrinkle);
			break;
		}
	}

	assert(desc.m_pPosition);
	assert(desc.m_pBoneWeight);
	assert(desc.m_pBoneMatrixIndex);
	assert(desc.m_pNormal);
	assert(desc.m_pColor);
	assert(desc.m_pSpecular);
	assert(desc.m_pTangentS);
	assert(desc.m_pTangentT);
	assert(desc.m_pWrinkle);
	assert(desc.m_pUserData);

	for (const auto& p : desc.m_pTexCoord)
		assert(p);
}

void IShaderAPI_MeshManager::ComputeVertexDescription(unsigned char* buffer, VertexFormat_t fmt, MeshDesc_t& desc) const
{
	LOG_FUNC();
	return ComputeVertexDescription((void*)buffer, VertexFormat(fmt), desc);
}

CMeshBuilder* IShaderAPI_MeshManager::GetVertexModifyBuilder()
{
	LOG_FUNC();
	return &m_MeshBuilder;
}
