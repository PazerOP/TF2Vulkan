#pragma once

#include "LogicalState.h"
#include "IShaderTextureManager.h"

#include <TF2Vulkan/Util/InPlaceVector.h>

#include <Color.h>
#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishaderdynamic.h>

#include <stack>

namespace TF2Vulkan
{
	class IShaderAPI_StateManagerDynamic : public IShaderTextureManager
	{
		using BaseClass = IShaderTextureManager;
	public:
		void PreDraw();

		void ClearColor3ub(uint8_t r, uint8_t g, uint8_t b) override final;
		void ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override final;

		void SetViewports(int count, const ShaderViewport_t* viewports) override final;

		void SetAnisotropicLevel(int anisoLevel) override final;
		void SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex) override final;
		void SetDefaultState() override final;
		int GetCurrentNumBones() const override final;
		void SetNumBoneWeights(int boneCount) override final;
		void Bind(IMaterial* material) override final;

		void MatrixMode(MaterialMatrixMode_t mode) override final;
		void GetMatrix(MaterialMatrixMode_t mode, float* dst) override final;
		void GetMatrix(MaterialMatrixMode_t mode, VMatrix& dst);
		void PushMatrix() override final;
		void PopMatrix() override final;
		void LoadIdentity() override final;
		void LoadMatrix(float* m) override final;
		void LoadMatrix(const VMatrix& m);

		bool InFlashlightMode() const override final;
		void BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t textureHandle) override final;

		void SetVertexShaderConstant(int var, const float* vec, int numConst, bool force = false) override final;
		void SetBooleanVertexShaderConstant(int var, const BOOL* vec, int numBools, bool force = false) override final;
		void SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force = false) override final;
		void SetPixelShaderConstant(int var, const float* vec, int numConst, bool force = false) override final;
		void SetBooleanPixelShaderConstant(int var, const BOOL* vec, int numBools, bool force = false) override final;
		void SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force = false) override final;

		void SetFloatRenderingParameter(RenderParamFloat_t param, float value) override final;
		void SetIntRenderingParameter(RenderParamInt_t param, int value) override final;
		void SetVectorRenderingParameter(RenderParamVector_t param, const Vector& value) override final;
		float GetFloatRenderingParameter(RenderParamFloat_t param) const override final;
		int GetIntRenderingParameter(RenderParamInt_t param) const override final;
		Vector GetVectorRenderingParameter(RenderParamVector_t param) const override final;

		void SetStencilEnable(bool enabled) override final;
		void SetStencilFailOperation(StencilOperation_t op) override final;
		void SetStencilZFailOperation(StencilOperation_t op) override final;
		void SetStencilPassOperation(StencilOperation_t op) override final;
		void SetStencilCompareFunction(StencilComparisonFunction_t fn) override final;
		void SetStencilReferenceValue(int ref) override final;
		void SetStencilTestMask(uint32 mask) override final;
		void SetStencilWriteMask(uint32 mask) override final;

		void FogStart(float start) override final { NOT_IMPLEMENTED_FUNC(); }
		void FogEnd(float end) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetFogZ(float fogZ) override final { NOT_IMPLEMENTED_FUNC(); }
		void SceneFogColor3ub(unsigned char r, unsigned char g, unsigned char b) override final { NOT_IMPLEMENTED_FUNC(); }
		void GetSceneFogColor(unsigned char* rgb) override final { NOT_IMPLEMENTED_FUNC(); }
		void SceneFogMode(MaterialFogMode_t mode) override final { NOT_IMPLEMENTED_FUNC(); }
		void GetFogDistances(float* start, float* end, float* fogZ) override final { NOT_IMPLEMENTED_FUNC(); }
		void FogMaxDensity(float maxDensity) override final { NOT_IMPLEMENTED_FUNC(); }
		MaterialFogMode_t GetSceneFogMode() override final;
		void SetPixelShaderFogParams(int reg) override final;
		MaterialFogMode_t GetCurrentFogType() const override final { NOT_IMPLEMENTED_FUNC(); }
		int GetPixelFogCombo() override final { NOT_IMPLEMENTED_FUNC_NOBREAK(); return 0; }

		void SetLight(int light, const LightDesc_t& desc) override final;
		void SetLightingOrigin(Vector lightingOrigin) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetAmbientLight(float r, float g, float b) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetAmbientLightCube(Vector4D cube[6]) override final;
		void SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack) override final;
		void CommitPixelShaderLighting(int pshReg) override final;
		void GetDX9LightState(LightState_t* state) const override final;

		void CullMode(MaterialCullMode_t mode) override final;

		void GetWorldSpaceCameraPosition(float* pos) const override final;

		void SetVertexShaderIndex(int index) override final;
		void SetPixelShaderIndex(int index) override final;

		void SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale) override final;

		void ForceDepthFuncEquals(bool enable) override final;

		void EnableLinearColorSpaceFrameBuffer(bool enable) override final;

		void SetToneMappingScaleLinear(const Vector& scale) override final;
		const Vector& GetToneMappingScaleLinear() const override final;

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
