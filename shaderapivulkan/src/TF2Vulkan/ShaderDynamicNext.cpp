#include "ShaderDynamicNext.h"

#include <TF2Vulkan/Util/DirtyVar.h>

#include <renderparm.h>

using namespace TF2Vulkan;

//static ShaderDynamicNext s_ShaderDynamic;

//IShaderDynamicNext* TF2Vulkan::g_ShaderDynamic = &s_ShaderDynamic;
void ShaderDynamicNext::SetViewports(int count, const ShaderViewport_t* viewports)
{
	LOG_FUNC();

	m_State.m_Viewports.clear();
	for (int i = 0; i < count; i++)
		m_State.m_Viewports.push_back(viewports[i]);

	m_Dirty = true;
}

MaterialFogMode_t ShaderDynamicNext::GetSceneFogMode()
{
	LOG_FUNC();
	return m_State.m_SceneFogMode;
}

void ShaderDynamicNext::SetPixelShaderFogParams(int reg)
{
	LOG_FUNC();
	float fogParams[4];

	NOT_IMPLEMENTED_FUNC_NOBREAK();

	SetPixelShaderConstant(reg, fogParams, 1);
}

void ShaderDynamicNext::SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	// TODO: Remove this function, we write directly into ps state via uniform buffers?
}

void ShaderDynamicNext::CommitPixelShaderLighting(int pshReg)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void ShaderDynamicNext::GetDX9LightState(LightState_t * state) const
{
	LOG_FUNC();

	if (state)
		* state = m_State.m_LightState;
}

void ShaderDynamicNext::GetWorldSpaceCameraPosition(float* pos) const
{
	LOG_FUNC();
	pos[0] = m_State.m_WorldSpaceCameraPosition.x;
	pos[1] = m_State.m_WorldSpaceCameraPosition.y;
	pos[2] = m_State.m_WorldSpaceCameraPosition.z;
}

void ShaderDynamicNext::SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void ShaderDynamicNext::SetToneMappingScaleLinear(const Vector & scale)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_TonemappingScale, scale, m_Dirty);
}

const Vector& ShaderDynamicNext::GetToneMappingScaleLinear() const
{
	LOG_FUNC();
	return m_State.m_TonemappingScale;
}

void ShaderDynamicNext::SetOverbright(float overbright)
{
	Util::SetDirtyVar(m_State.m_SCOverbright, overbright, m_Dirty);
}

void ShaderDynamicNext::SetDefaultState()
{
	LOG_FUNC();
	//m_State = {};
	m_Dirty = true;
}

int ShaderDynamicNext::GetCurrentNumBones() const
{
	LOG_FUNC();
	return m_State.m_BoneCount;
}

void ShaderDynamicNext::MatrixMode(MaterialMatrixMode_t mode)
{
	LOG_FUNC();

	ENSURE(mode >= 0);
	ENSURE(mode < NUM_MATRIX_MODES);

	m_MatrixMode = mode;
}

void ShaderDynamicNext::GetMatrix(MaterialMatrixMode_t mode, float* dst)
{
	return GetMatrix(mode, *reinterpret_cast<VMatrix*>(dst));
}

void ShaderDynamicNext::GetMatrix(MaterialMatrixMode_t mode, VMatrix & dst)
{
	LOG_FUNC();
	dst = m_State.m_Matrices.at(mode);
}

void ShaderDynamicNext::AssertMatrixMode()
{
	assert(m_MatrixMode >= 0);
	assert(m_MatrixMode < NUM_MATRIX_MODES);
}

void ShaderDynamicNext::PushMatrix()
{
	LOG_FUNC();

	auto& stack = m_MatrixStacks.at(m_MatrixMode);
	auto& mat = m_State.m_Matrices.at(m_MatrixMode);
	stack.emplace(mat);
}

void ShaderDynamicNext::PopMatrix()
{
	LOG_FUNC();

	auto& mat = m_State.m_Matrices.at(m_MatrixMode);
	auto& stack = m_MatrixStacks.at(m_MatrixMode);

	mat = stack.top();
	m_Dirty = true;

	stack.pop();
}

void ShaderDynamicNext::LoadIdentity()
{
	LOG_FUNC();

	m_State.m_Matrices.at(m_MatrixMode).Identity();
	m_Dirty = true;
}

void ShaderDynamicNext::LoadMatrix(float* m)
{
	return LoadMatrix(*reinterpret_cast<const VMatrix*>(m));
}

void ShaderDynamicNext::LoadMatrix(const VMatrix & m)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_Matrices.at(m_MatrixMode), m, m_Dirty);
}

bool ShaderDynamicNext::InFlashlightMode() const
{
	LOG_FUNC();
	return m_State.m_InFlashlightMode;
}

void ShaderDynamicNext::SetVertexShaderConstant(int var, const float* vec, int numConst, bool force)
{
	NOT_IMPLEMENTED_FUNC();
#if false
	LOG_FUNC();
	g_StateManagerStatic.GetVertexShader().GetCompatData().SetConstants(
		m_State.m_ShaderData.m_VSData, Util::SafeConvert<uint32_t>(var),
		reinterpret_cast<const ShaderConstants::float4*>(vec),
		Util::SafeConvert<uint32_t>(numConst));
#endif
}

void ShaderDynamicNext::SetBooleanVertexShaderConstant(int var, const BOOL * vec, int numBools, bool force)
{
	NOT_IMPLEMENTED_FUNC();
#if false
	LOG_FUNC();
	g_StateManagerStatic.GetVertexShader().GetCompatData().SetConstants(
		m_State.m_ShaderData.m_VSData, Util::SafeConvert<uint32_t>(var),
		reinterpret_cast<const ShaderConstants::bool4*>(vec),
		Util::SafeConvert<uint32_t>(numBools));
#endif
}

void ShaderDynamicNext::SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force)
{
	NOT_IMPLEMENTED_FUNC();
#if false
	LOG_FUNC();
	g_StateManagerStatic.GetVertexShader().GetCompatData().SetConstants(
		m_State.m_ShaderData.m_VSData, Util::SafeConvert<uint32_t>(var),
		reinterpret_cast<const ShaderConstants::int4*>(vec),
		Util::SafeConvert<uint32_t>(numIntVecs));
#endif
}

void ShaderDynamicNext::SetPixelShaderConstant(int var, const float* vec, int numVecs, bool force)
{
	NOT_IMPLEMENTED_FUNC();
#if false
	LOG_FUNC();
	auto& ps = g_StateManagerStatic.GetPixelShader();
	[[maybe_unused]] auto dbgName = ps.GetName();
	ps.GetCompatData().SetConstants(
		m_State.m_ShaderData.m_PSData, Util::SafeConvert<uint32_t>(var),
		reinterpret_cast<const ShaderConstants::float4*>(vec),
		Util::SafeConvert<uint32_t>(numVecs));
#endif
}

void ShaderDynamicNext::SetBooleanPixelShaderConstant(int var, const BOOL * vec, int numBools, bool force)
{
	NOT_IMPLEMENTED_FUNC();
#if false
	LOG_FUNC();
	g_StateManagerStatic.GetPixelShader().GetCompatData().SetConstants(
		m_State.m_ShaderData.m_PSData, Util::SafeConvert<uint32_t>(var),
		reinterpret_cast<const ShaderConstants::bool4*>(vec),
		Util::SafeConvert<uint32_t>(numBools));
#endif
}

void ShaderDynamicNext::SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force)
{
	NOT_IMPLEMENTED_FUNC();
#if false
	LOG_FUNC();
	g_StateManagerStatic.GetPixelShader().GetCompatData().SetConstants(
		m_State.m_ShaderData.m_PSData, Util::SafeConvert<uint32_t>(var),
		reinterpret_cast<const ShaderConstants::int4*>(vec),
		Util::SafeConvert<uint32_t>(numIntVecs));
#endif
}

void ShaderDynamicNext::SetFloatRenderingParameter(RenderParamFloat_t param, float value)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_RenderParamsFloat, param, value, m_Dirty);
}

void ShaderDynamicNext::SetIntRenderingParameter(RenderParamInt_t param, int value)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_RenderParamsInt, param, value, m_Dirty);
}

void ShaderDynamicNext::SetVectorRenderingParameter(RenderParamVector_t param, const Vector & value)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_RenderParamsVector, param, value, m_Dirty);
}

float ShaderDynamicNext::GetFloatRenderingParameter(RenderParamFloat_t param) const
{
	LOG_FUNC();
	return m_State.m_RenderParamsFloat.at(param);
}

int ShaderDynamicNext::GetIntRenderingParameter(RenderParamInt_t param) const
{
	LOG_FUNC();
	return m_State.m_RenderParamsInt.at(param);
}

Vector ShaderDynamicNext::GetVectorRenderingParameter(RenderParamVector_t param) const
{
	LOG_FUNC();
	return m_State.m_RenderParamsVector.at(param);
}

void ShaderDynamicNext::SetStencilEnable(bool enabled)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilEnable, enabled, m_Dirty);
}

void ShaderDynamicNext::SetStencilFailOperation(StencilOperation_t op)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilFailOp, op, m_Dirty);
}

void ShaderDynamicNext::SetStencilZFailOperation(StencilOperation_t op)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilDepthFailOp, op, m_Dirty);
}

void ShaderDynamicNext::SetStencilPassOperation(StencilOperation_t op)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilPassOp, op, m_Dirty);
}

void ShaderDynamicNext::SetStencilCompareFunction(StencilComparisonFunction_t fn)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilCompareFunc, fn, m_Dirty);
}

void ShaderDynamicNext::SetStencilReferenceValue(int ref)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilRef, ref, m_Dirty);
}

void ShaderDynamicNext::SetStencilTestMask(uint32 mask)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilTestMask, mask, m_Dirty);
}

void ShaderDynamicNext::SetStencilWriteMask(uint32 mask)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilWriteMask, mask, m_Dirty);
}
