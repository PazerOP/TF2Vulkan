#include "stdafx.h"
#include "IStateManagerDynamic.h"
#include "interface/IMaterialInternal.h"
#include "interface/internal/IStateManagerStatic.h"
#include "shaders/VulkanShaderManager.h"

#include <TF2Vulkan/Util/DirtyVar.h>

#include <materialsystem/IShader.h>


using namespace TF2Vulkan;

void IShaderAPI_StateManagerDynamic::SetAnisotropicLevel(int anisoLevel)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_AnisotropicLevel, anisoLevel, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetLight(int light, const LightDesc_t& desc)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_Lights, light, desc, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetAmbientLightCube(Vector4D cube[6])
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_LightAmbientCube,
		*reinterpret_cast<const std::array<Vector4D, 6>*>(cube), m_Dirty);
}

void IShaderAPI_StateManagerDynamic::CullMode(MaterialCullMode_t mode)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_CullMode, mode, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::ForceDepthFuncEquals(bool enable)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_ForceDepthFuncEquals, enable, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::EnableLinearColorSpaceFrameBuffer(bool enable)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_FBLinear, enable, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_FullScreenTexture, tex, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetNumBoneWeights(int boneCount)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_BoneCount, boneCount, m_Dirty); // TODO: <# bone weights> == <bone count>?
}

void IShaderAPI_StateManagerDynamic::ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	LOG_FUNC();

	float fValues[4] =
	{
		r / 255.0f,
		g / 255.0f,
		b / 255.0f,
		a / 255.0f,
	};

	for (uint_fast8_t i = 0; i < 4; i++)
		Util::SetDirtyVar(m_State.m_ClearColor, i, fValues[i], m_Dirty);
}

void IShaderAPI_StateManagerDynamic::PreDraw()
{
	auto& vsData = m_State.m_ShaderData.m_VSData;
	const auto viewProj = m_State.m_Matrices.at(MATERIAL_PROJECTION) * m_State.m_Matrices.at(MATERIAL_VIEW);
	vsData.m_Matrices.m_ViewProj.SetFrom(viewProj.Transpose());

	const auto& modelMatrix = m_State.m_Matrices.at(MATERIAL_MODEL);
	vsData.m_ModelMatrices.m_Model[0] = modelMatrix.Transpose().As3x4();

	const auto modelViewProj = viewProj * modelMatrix;
	vsData.m_Matrices.m_ModelViewProj.SetFrom(modelViewProj.Transpose());
}

void IShaderAPI_StateManagerDynamic::ClearColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
	LOG_FUNC();
	return ClearColor4ub(r, g, b, 255);
}

void IShaderAPI_StateManagerDynamic::Bind(IMaterial* material)
{
	LOG_FUNC();

	auto internal = assert_cast<IMaterialInternal*>(material);
	auto isPrecached = internal->IsPrecached();
	auto refCount = internal->GetReferenceCount();
	auto needsWhiteLightmap = internal->GetNeedsWhiteLightmap();
	auto minLightmapPageID = internal->GetMinLightmapPageID();
	auto maxLightmapPageID = internal->GetMaxLightmapPageID();
	IShader* shader = internal->GetShader();
	auto isPrecachedVars = internal->IsPrecachedVars();
	auto vertexUsage = VertexFormat(internal->GetVertexUsage());

	Util::SetDirtyVar(m_State.m_BoundMaterial, material, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t textureHandle)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_BoundTextures, sampler, textureHandle, m_Dirty);
}
