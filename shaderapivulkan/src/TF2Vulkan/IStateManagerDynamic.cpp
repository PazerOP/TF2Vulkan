#include "stdafx.h"
#include "IStateManagerDynamic.h"
#include "interface/IMaterialInternal.h"
#include "interface/internal/IShaderInternal.h"
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

void IShaderAPI_StateManagerDynamic::SetViewports(int count, const ShaderViewport_t* viewports)
{
	LOG_FUNC();

	m_State.m_Viewports.clear();
	for (int i = 0; i < count; i++)
		m_State.m_Viewports.push_back(viewports[i]);

	m_Dirty = true;
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

void IShaderAPI_StateManagerDynamic::SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	// TODO: Remove this function, we write directly into ps state via uniform buffers?
}

void IShaderAPI_StateManagerDynamic::CommitPixelShaderLighting(int pshReg)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void IShaderAPI_StateManagerDynamic::GetDX9LightState(LightState_t * state) const
{
	LOG_FUNC();

	if (state)
		* state = m_State.m_LightState;
}

void IShaderAPI_StateManagerDynamic::GetWorldSpaceCameraPosition(float* pos) const
{
	assert(uintptr_t(pos) % alignof(Vector) == 0); // Alignment must match
#pragma warning(suppress : 4996)
	return GetWorldSpaceCameraPosition(*reinterpret_cast<Vector*>(pos));
}

void IShaderAPI_StateManagerDynamic::GetWorldSpaceCameraPosition(Vector& pos) const
{
	LOG_FUNC();
	pos[0] = m_State.m_WorldSpaceCameraPosition.x;
	pos[1] = m_State.m_WorldSpaceCameraPosition.y;
	pos[2] = m_State.m_WorldSpaceCameraPosition.z;
}

void IShaderAPI_StateManagerDynamic::SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void IShaderAPI_StateManagerDynamic::SetToneMappingScaleLinear(const Vector & scale)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_TonemappingScale, scale, m_Dirty);
}

const Vector& IShaderAPI_StateManagerDynamic::GetToneMappingScaleLinear() const
{
	LOG_FUNC();
	return m_State.m_TonemappingScale;
}

void IShaderAPI_StateManagerDynamic::SetShaderInstance(ShaderType type, const IShaderInstance* instance)
{
	switch (type)
	{
	case ShaderType::Pixel:
		Util::SetDirtyVar(m_State.m_PSShader, assert_cast<const IShaderInstanceInternal*>(instance), m_Dirty);
		break;
	case ShaderType::Vertex:
		Util::SetDirtyVar(m_State.m_VSShader, assert_cast<const IShaderInstanceInternal*>(instance), m_Dirty);
		break;

	default:
		assert(!"Unknown shader type");
		break;
	}
}

void IShaderAPI_StateManagerDynamic::BindUniformBuffer(UniformBuffer& buf, UniformBufferIndex index)
{
	if (index == UniformBufferIndex::Invalid)
		return; // Nothing to do

	Util::SetDirtyVar(m_State.m_UniformBuffers, index, buf, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetOverbright(float overbright)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	//Util::SetDirtyVar(m_State.m_SCOverbright, overbright, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetDefaultState()
{
	LOG_FUNC();
	//m_State = {};
	m_Dirty = true;
}

int IShaderAPI_StateManagerDynamic::GetCurrentNumBones() const
{
	LOG_FUNC();
	return m_State.m_BoneCount;
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
	return GetMatrix(mode, *reinterpret_cast<VMatrix*>(dst));
}

void IShaderAPI_StateManagerDynamic::GetMatrix(MaterialMatrixMode_t mode, VMatrix & dst) const
{
	LOG_FUNC();
	dst = m_State.m_Matrices.at(mode);
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

void IShaderAPI_StateManagerDynamic::LoadMatrix(float* m)
{
	LOG_FUNC();
	return LoadMatrix(*reinterpret_cast<const VMatrix*>(m));
}

void IShaderAPI_StateManagerDynamic::LoadMatrix(const VMatrix & m)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_Matrices.at(m_MatrixMode), m, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::MultMatrix(float* m)
{
	LOG_FUNC();
	return MultMatrix(*reinterpret_cast<const VMatrix*>(m));
}

void IShaderAPI_StateManagerDynamic::MultMatrixLocal(float* m)
{
	LOG_FUNC();
	return MultMatrixLocal(*reinterpret_cast<const VMatrix*>(m));
}

bool IShaderAPI_StateManagerDynamic::InFlashlightMode() const
{
	LOG_FUNC();
	return m_State.m_InFlashlightMode;
}

void IShaderAPI_StateManagerDynamic::SetVertexShaderConstant(int var, const float* vec, int numConst, bool force)
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

void IShaderAPI_StateManagerDynamic::SetBooleanVertexShaderConstant(int var, const BOOL * vec, int numBools, bool force)
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

void IShaderAPI_StateManagerDynamic::SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force)
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

void IShaderAPI_StateManagerDynamic::SetPixelShaderConstant(int var, const float* vec, int numVecs, bool force)
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

void IShaderAPI_StateManagerDynamic::SetBooleanPixelShaderConstant(int var, const BOOL * vec, int numBools, bool force)
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

void IShaderAPI_StateManagerDynamic::SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force)
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

void IShaderAPI_StateManagerDynamic::SetFloatRenderingParameter(RenderParamFloat_t param, float value)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_RenderParamsFloat, param, value, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetIntRenderingParameter(RenderParamInt_t param, int value)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_RenderParamsInt, param, value, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetVectorRenderingParameter(RenderParamVector_t param, const Vector & value)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_RenderParamsVector, param, value, m_Dirty);
}

float IShaderAPI_StateManagerDynamic::GetFloatRenderingParameter(RenderParamFloat_t param) const
{
	LOG_FUNC();
	return m_State.m_RenderParamsFloat.at(param);
}

int IShaderAPI_StateManagerDynamic::GetIntRenderingParameter(RenderParamInt_t param) const
{
	LOG_FUNC();
	return m_State.m_RenderParamsInt.at(param);
}

Vector IShaderAPI_StateManagerDynamic::GetVectorRenderingParameter(RenderParamVector_t param) const
{
	LOG_FUNC();
	return m_State.m_RenderParamsVector.at(param);
}

void IShaderAPI_StateManagerDynamic::SetStencilEnable(bool enabled)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilEnable, enabled, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilFailOperation(StencilOperation_t op)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilFailOp, op, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilZFailOperation(StencilOperation_t op)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilDepthFailOp, op, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilPassOperation(StencilOperation_t op)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilPassOp, op, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilCompareFunction(StencilComparisonFunction_t fn)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilCompareFunc, fn, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilReferenceValue(int ref)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilRef, ref, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilTestMask(uint32 mask)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilTestMask, mask, m_Dirty);
}

void IShaderAPI_StateManagerDynamic::SetStencilWriteMask(uint32 mask)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_State.m_StencilWriteMask, mask, m_Dirty);
}
