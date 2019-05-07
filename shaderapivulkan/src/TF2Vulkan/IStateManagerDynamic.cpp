#include "stdafx.h"
#include "IStateManagerDynamic.h"
#include "interface/IMaterialInternal.h"

#include <TF2Vulkan/Util/DirtyVar.h>

#include <materialsystem/IShader.h>

using namespace TF2Vulkan;

void IShaderAPI_StateManagerDynamic::SetViewports(int count, const ShaderViewport_t* viewports)
{
	LOG_FUNC();

	m_State.m_Viewports.clear();
	for (int i = 0; i < count; i++)
		m_State.m_Viewports.push_back(viewports[i]);

	m_Dirty = true;
}

void IShaderAPI_StateManagerDynamic::SetAnisotropicLevel(int anisoLevel)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_AnisotropicLevel, anisoLevel, m_Dirty);
}

MaterialFogMode_t IShaderAPI_StateManagerDynamic::GetSceneFogMode()
{
	LOG_FUNC();
	return m_State.m_SceneFogMode;
}

void IShaderAPI_StateManagerDynamic::SetPixelShaderFogParams(int reg)
{
	LOG_FUNC();
	float fogParams[4];

	NOT_IMPLEMENTED_FUNC_NOBREAK();

	SetPixelShaderConstant(reg, fogParams, 1);
}

void IShaderAPI_StateManagerDynamic::GetWorldSpaceCameraPosition(float* pos) const
{
	LOG_FUNC();
	pos[0] = m_State.m_WorldSpaceCameraPosition.x;
	pos[1] = m_State.m_WorldSpaceCameraPosition.y;
	pos[2] = m_State.m_WorldSpaceCameraPosition.z;
}

void IShaderAPI_StateManagerDynamic::SetVertexShaderIndex(int index)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_VertexShaderIndex, index, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetPixelShaderIndex(int index)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_PixelShaderIndex, index, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::ForceDepthFuncEquals(bool enable)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_ForceDepthFuncEquals, enable, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetOverbright(float overbright)
{
	Util::SetDirtyVar(m_State.m_SCOverbright, overbright, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_FullScreenTexture, tex, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetDefaultState()
{
	LOG_FUNC();
	m_State = {};
	m_Dirty = true;
}

int IShaderAPI_StateManagerDynamic::GetCurrentNumBones() const
{
	LOG_FUNC();
	return m_State.m_BoneCount;
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

void IShaderAPI_StateManagerDynamic::MatrixMode(MaterialMatrixMode_t mode)
{
	LOG_FUNC();

	ENSURE(mode >= 0);
	ENSURE(mode < NUM_MATRIX_MODES);

	m_MatrixMode = mode;
}

void IShaderAPI_StateManagerDynamic::GetMatrix(MaterialMatrixMode_t mode, float* dst)
{
	LOG_FUNC();
	memcpy(dst, m_State.m_Matrices.at(mode).Base(), sizeof(float) * 4 * 4);
}

void IShaderAPI_StateManagerDynamic::AssertMatrixMode()
{
	assert(m_MatrixMode >= 0);
	assert(m_MatrixMode < NUM_MATRIX_MODES);
}

void IShaderAPI_StateManagerDynamic::PushMatrix()
{
	LOG_FUNC();

	auto& stack = m_MatrixStacks.at(m_MatrixMode);
	auto& mat = m_State.m_Matrices.at(m_MatrixMode);
	stack.emplace(mat);
}

void IShaderAPI_StateManagerDynamic::PopMatrix()
{
	LOG_FUNC();

	auto& mat = m_State.m_Matrices.at(m_MatrixMode);
	auto& stack = m_MatrixStacks.at(m_MatrixMode);

	mat = stack.top();
	m_Dirty = true;

	stack.pop();
}

void IShaderAPI_StateManagerDynamic::LoadIdentity()
{
	LOG_FUNC();

	m_State.m_Matrices.at(m_MatrixMode).Identity();
	m_Dirty = true;
}

bool IShaderAPI_StateManagerDynamic::InFlashlightMode() const
{
	LOG_FUNC();
	return m_State.m_InFlashlightMode;
}

void IShaderAPI_StateManagerDynamic::BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t textureHandle)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_BoundTextures, sampler, textureHandle, m_Dirty);
}

template<typename T>
static void SetShaderConstantsSafe(ShaderConstantValues& constants, int var, const T* vec, int numConst)
{
	constants.SetData(
		Util::SafeConvert<size_t>(var),
		reinterpret_cast<const ShaderConstant*>(vec),
		Util::SafeConvert<size_t>(numConst));
}

void IShaderAPI_StateManagerDynamic::SetVertexShaderConstant(int var, const float* vec, int numConst, bool force)
{
	LOG_FUNC();
	SetShaderConstantsSafe(m_State.m_VertexShaderConstants, var, vec, numConst);
}

void IShaderAPI_StateManagerDynamic::SetBooleanVertexShaderConstant(int var, const BOOL* vec, int numBools, bool force)
{
	LOG_FUNC();
	SetShaderConstantsSafe(m_State.m_VertexShaderConstants, var, vec, numBools);
}

void IShaderAPI_StateManagerDynamic::SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force)
{
	LOG_FUNC();
	SetShaderConstantsSafe(m_State.m_VertexShaderConstants, var, vec, numIntVecs);
}

void IShaderAPI_StateManagerDynamic::SetPixelShaderConstant(int var, const float* vec, int numConst, bool force)
{
	LOG_FUNC();
	SetShaderConstantsSafe(m_State.m_PixelShaderConstants, var, vec, numConst);
}

void IShaderAPI_StateManagerDynamic::SetBooleanPixelShaderConstant(int var, const BOOL* vec, int numBools, bool force)
{
	LOG_FUNC();
	SetShaderConstantsSafe(m_State.m_PixelShaderConstants, var, vec, numBools);
}

void IShaderAPI_StateManagerDynamic::SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force)
{
	LOG_FUNC();
	SetShaderConstantsSafe(m_State.m_PixelShaderConstants, var, vec, numIntVecs);
}
