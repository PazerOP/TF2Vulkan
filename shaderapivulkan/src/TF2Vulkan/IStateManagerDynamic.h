#pragma once

#include "LogicalState.h"
#include "IShaderTextureManager.h"
#include "ShaderDynamicNext.h"

#include <TF2Vulkan/Util/ForwardingHelpers.h>
#include <TF2Vulkan/Util/InPlaceVector.h>

#include <Color.h>
#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishaderdynamic.h>

#include <stack>

#pragma push_macro("FORWARD_FN_IMPL")
#undef FORWARD_FN_IMPL
#define FORWARD_FN_IMPL(name, ...) g_ShaderDynamic->name(__VA_ARGS__)

namespace TF2Vulkan
{
	class IShaderAPI_StateManagerDynamic : public IShaderTextureManager
	{
		using BaseClass = IShaderTextureManager;
	public:
		void PreDraw();

		void ClearColor3ub(uint8_t r, uint8_t g, uint8_t b) override final;
		void ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override final;

		void SetAnisotropicLevel(int anisoLevel) override final;
		void SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex) override final;
		//void SetDefaultState() override final;
		//int GetCurrentNumBones() const override final;
		void SetNumBoneWeights(int boneCount) override final;
		void Bind(IMaterial* material) override final;

		//bool InFlashlightMode() const override final;
		void BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t textureHandle) override final;

		//void SetVertexShaderConstant(int var, const float* vec, int numConst, bool force = false) override final;
		//void SetBooleanVertexShaderConstant(int var, const BOOL* vec, int numBools, bool force = false) override final;
		//void SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force = false) override final;
		//void SetPixelShaderConstant(int var, const float* vec, int numConst, bool force = false) override final;
		//void SetBooleanPixelShaderConstant(int var, const BOOL* vec, int numBools, bool force = false) override final;
		//void SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force = false) override final;

		//void SetFloatRenderingParameter(RenderParamFloat_t param, float value) override final;
		//void SetIntRenderingParameter(RenderParamInt_t param, int value) override final;
		//void SetVectorRenderingParameter(RenderParamVector_t param, const Vector& value) override final;
		//float GetFloatRenderingParameter(RenderParamFloat_t param) const override final;
		//int GetIntRenderingParameter(RenderParamInt_t param) const override final;
		//Vector GetVectorRenderingParameter(RenderParamVector_t param) const override final;

		//void SetStencilEnable(bool enabled) override final;
		//void SetStencilFailOperation(StencilOperation_t op) override final;
		//void SetStencilZFailOperation(StencilOperation_t op) override final;
		//void SetStencilPassOperation(StencilOperation_t op) override final;
		//void SetStencilCompareFunction(StencilComparisonFunction_t fn) override final;
		//void SetStencilReferenceValue(int ref) override final;
		//void SetStencilTestMask(uint32 mask) override final;
		//void SetStencilWriteMask(uint32 mask) override final;

		void FogStart(float start) override final { NOT_IMPLEMENTED_FUNC(); }
		void FogEnd(float end) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetFogZ(float fogZ) override final { NOT_IMPLEMENTED_FUNC(); }
		void SceneFogColor3ub(unsigned char r, unsigned char g, unsigned char b) override final { NOT_IMPLEMENTED_FUNC(); }
		void SceneFogMode(MaterialFogMode_t mode) override final { NOT_IMPLEMENTED_FUNC(); }
		void GetFogDistances(float* start, float* end, float* fogZ) override final { NOT_IMPLEMENTED_FUNC(); }
		void FogMaxDensity(float maxDensity) override final { NOT_IMPLEMENTED_FUNC(); }
		//MaterialFogMode_t GetSceneFogMode() override final;
		//void SetPixelShaderFogParams(int reg) override final;
		//MaterialFogMode_t GetCurrentFogType() const override final { NOT_IMPLEMENTED_FUNC(); }
		int GetPixelFogCombo() override final { NOT_IMPLEMENTED_FUNC_NOBREAK(); return 0; }

		void SetLight(int light, const LightDesc_t& desc) override final;
		void SetLightingOrigin(Vector lightingOrigin) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetAmbientLight(float r, float g, float b) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetAmbientLightCube(Vector4D cube[6]) override final;
		//void SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack) override final;
		//void CommitPixelShaderLighting(int pshReg) override final;
		//void GetDX9LightState(LightState_t* state) const override final;

		void CullMode(MaterialCullMode_t mode) override final;

		//void GetWorldSpaceCameraPosition(float* pos) const override final;

		//void SetVertexShaderIndex(int index) override final;
		//void SetPixelShaderIndex(int index) override final;

		//void SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale) override final;

		void ForceDepthFuncEquals(bool enable) override final;

		void EnableLinearColorSpaceFrameBuffer(bool enable) override final;

		//void SetToneMappingScaleLinear(const Vector& scale) override final;
		//const Vector& GetToneMappingScaleLinear() const override final;

		// Forwarded to IShaderDynamicNext
		// NOTE: I can't wait to rewrite MaterialSystem so we can fix this god awful
		// class layout... WHY does IShaderAPI inherit from IShaderDynamicAPI???
		FORWARD_FN(SetViewports, 2, int, count, const ShaderViewport_t*, viewports);
		FORWARD_FN_CONST(GetViewports, 2, ShaderViewport_t*, viewports, int, maxViewports);
		FORWARD_FN_CONST(CurrentTime, 0);
		FORWARD_FN(GetLightmapDimensions, 2, int*, w, int*, h);
		FORWARD_FN(GetSceneFogMode, 0);
		FORWARD_FN(GetSceneFogColor, 1, unsigned char*, rgb);
		FORWARD_FN(MatrixMode, 1, MaterialMatrixMode_t, matrixMode);
		FORWARD_FN(PushMatrix, 0);
		FORWARD_FN(PopMatrix, 0);
		FORWARD_FN(LoadMatrix, 1, float*, m);
		FORWARD_FN(MultMatrix, 1, float*, m);
		FORWARD_FN(MultMatrixLocal, 1, float*, m);
		FORWARD_FN(GetMatrix, 2, MaterialMatrixMode_t, matrixMode, float*, dst);
		FORWARD_FN(LoadIdentity, 0);
		FORWARD_FN(LoadCameraToWorld, 0);
		FORWARD_FN(Ortho, 6, double, left, double, right, double, bottom, double, top, double, zNear, double, zFar);
		FORWARD_FN(PerspectiveX, 4, double, fovx, double, aspect, double, zNear, double, zFar);
		FORWARD_FN(PickMatrix, 4, int, x, int, y, int, width, int, height);
		FORWARD_FN(Rotate, 4, float, angle, float, x, float, y, float, z);
		FORWARD_FN(Translate, 3, float, x, float, y, float, z);
		FORWARD_FN(Scale, 3, float, x, float, y, float, z);
		FORWARD_FN(ScaleXY, 2, float, x, float, y);

		FORWARD_FN(Color3f, 3, float, r, float, g, float, b);
		FORWARD_FN(Color3fv, 1, const float*, pColor);
		FORWARD_FN(Color4f, 4, float, r, float, g, float, b, float, a);
		FORWARD_FN(Color4fv, 1, const float*, pColor);

		FORWARD_FN(Color3ub, 3, unsigned char, r, unsigned char, g, unsigned char, b);
		FORWARD_FN(Color3ubv, 1, unsigned const char*, pColor);
		FORWARD_FN(Color4ub, 4, unsigned char, r, unsigned char, g, unsigned char, b, unsigned char, a);
		FORWARD_FN(Color4ubv, 1, unsigned const char*, pColor);

		FORWARD_FN(SetVertexShaderConstant, 4, int, var, const float*, vec, int, numConst, bool, force);
		FORWARD_FN(SetPixelShaderConstant, 4, int, var, const float*, vec, int, numConst, bool, force);

		FORWARD_FN(SetDefaultState, 0);

		FORWARD_FN_CONST(GetWorldSpaceCameraPosition, 1, float*, pos);

		FORWARD_FN_CONST(GetCurrentNumBones, 0);
		FORWARD_FN_CONST(GetCurrentLightCombo, 0);

		FORWARD_FN_CONST(GetCurrentFogType, 0);

		FORWARD_FN(SetTextureTransformDimension, 3, TextureStage_t, textureStage, int, dimension, bool, projected);
		FORWARD_FN(DisableTextureTransform, 1, TextureStage_t, textureStage);
		FORWARD_FN(SetBumpEnvMatrix, 5, TextureStage_t, textureStage, float, m00, float, m01, float, m10, float, m11);

		FORWARD_FN(SetVertexShaderIndex, 1, int, vshIndex);
		FORWARD_FN(SetPixelShaderIndex, 1, int, pshIndex);

		FORWARD_FN_CONST(GetBackBufferDimensions, 2, int&, width, int&, height);

		// Helpers
		void SetOverbright(float overbright);

		const LogicalDynamicState& GetDynamicState() const { return m_State; }

	private:
		void AssertMatrixMode();

		MaterialMatrixMode_t m_MatrixMode = {};
		std::array<std::stack<VMatrix, std::vector<VMatrix>>, NUM_MATRIX_MODES> m_MatrixStacks;

		LogicalDynamicState m_State;
		bool m_Dirty = true;
	};

	extern IShaderAPI_StateManagerDynamic& g_StateManagerDynamic;
}
#pragma pop_macro("FORWARD_FN_IMPL")
