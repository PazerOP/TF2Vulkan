#include "stdafx.h"
#include "IStateManagerDynamic.h"

#include <TF2Vulkan/Util/DirtyVar.h>

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
	Util::SetDirtyVar(m_State.m_BoundMaterial, material, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::MatrixMode(MaterialMatrixMode_t mode)
{
	LOG_FUNC();

	ENSURE(mode >= 0);
	ENSURE(mode < NUM_MATRIX_MODES);

	m_MatrixMode = mode;
}

void IShaderAPI_StateManagerDynamic::AssertMatrixMode()
{
	assert(m_MatrixMode >= 0);
	assert(m_MatrixMode < NUM_MATRIX_MODES);
}

void IShaderAPI_StateManagerDynamic::PushMatrix()
{
	LOG_FUNC();

	AssertMatrixMode();

	auto& mat = m_State.m_Matrices[m_MatrixMode];
	mat.emplace();
	m_Dirty = true;
}

void IShaderAPI_StateManagerDynamic::PopMatrix()
{
	LOG_FUNC();

	AssertMatrixMode();

	auto& mat = m_State.m_Matrices[m_MatrixMode];
	mat.pop();
	m_Dirty = true;
}

void IShaderAPI_StateManagerDynamic::LoadIdentity()
{
	LOG_FUNC();

	AssertMatrixMode();

	auto& mat = m_State.m_Matrices[m_MatrixMode];
	mat.top().Identity();
	m_Dirty = true;
}
