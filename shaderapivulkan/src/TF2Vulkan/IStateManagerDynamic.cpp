#include "stdafx.h"
#include "IStateManagerDynamic.h"

#include <TF2Vulkan/Util/DirtyVar.h>

using namespace TF2Vulkan;

void IStateManagerDynamic::SetViewports(int count, const ShaderViewport_t* viewports)
{
	LOG_FUNC();

	m_State.m_Viewports.clear();
	for (int i = 0; i < count; i++)
		m_State.m_Viewports.push_back(viewports[i]);

	m_Dirty = true;
}

void IStateManagerDynamic::SetAnisotropicLevel(int anisoLevel)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_AnisotropicLevel, anisoLevel, m_Dirty);
}

void IStateManagerDynamic::SetOverbright(float overbright)
{
	Util::SetDirtyVar(m_State.m_SCOverbright, overbright, m_Dirty);
}

void IStateManagerDynamic::SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_FullScreenTexture, tex, m_Dirty);
}

void IStateManagerDynamic::SetDefaultState()
{
	LOG_FUNC();
	m_State = {};
	m_Dirty = true;
}

int IStateManagerDynamic::GetCurrentNumBones() const
{
	LOG_FUNC();
	return m_State.m_BoneCount;
}

void IStateManagerDynamic::ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
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

void IStateManagerDynamic::ClearColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
	LOG_FUNC();
	return ClearColor4ub(r, g, b, 255);
}

void IStateManagerDynamic::Bind(IMaterial* material)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_BoundMaterial, material, m_Dirty);
}
