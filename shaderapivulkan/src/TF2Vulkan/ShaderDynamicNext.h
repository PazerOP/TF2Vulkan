#pragma once
#include "LogicalState.h"

#include <TF2Vulkan/IShaderDynamicNext.h>

#include <TF2Vulkan/Util/std_stack.h>

namespace TF2Vulkan
{
	class ShaderDynamicNext : public IShaderDynamicNext
	{
	public:
		void SetViewports(int count, const ShaderViewport_t* viewports) override final;
		int GetViewports(ShaderViewport_t* viewports, int max) const override final { NOT_IMPLEMENTED_FUNC(); }

		double CurrentTime() const override final { NOT_IMPLEMENTED_FUNC(); }

		void GetLightmapDimensions(int* w, int* h) override final { NOT_IMPLEMENTED_FUNC(); }

		void SetDefaultState() override final;
		int GetCurrentNumBones() const override final;

		void MatrixMode(MaterialMatrixMode_t mode) override final;
		void PushMatrix() override final;
		void PopMatrix() override final;
		void LoadMatrix(float* m) override final;
		void LoadMatrix(const VMatrix& m);
		void MultMatrix(float* m) override final { NOT_IMPLEMENTED_FUNC(); }
		void MultMatrix(const VMatrix& m) { NOT_IMPLEMENTED_FUNC(); }
		void MultMatrixLocal(float* m) override final { NOT_IMPLEMENTED_FUNC(); }
		void MultMatrixLocal(const VMatrix& m) { NOT_IMPLEMENTED_FUNC(); }
		void GetMatrix(MaterialMatrixMode_t mode, float* dst) override final;
		void GetMatrix(MaterialMatrixMode_t mode, VMatrix& dst);
		void LoadIdentity() override final;
		void LoadCameraToWorld() override final { NOT_IMPLEMENTED_FUNC(); }
		void Ortho(double left, double right, double bottom, double top, double zNear, double zFar) override final { NOT_IMPLEMENTED_FUNC(); }
		void PerspectiveX(double fovx, double aspect, double zNear, double zFar) override final { NOT_IMPLEMENTED_FUNC(); }
		void PickMatrix(int x, int y, int width, int height) override final { NOT_IMPLEMENTED_FUNC(); }
		void Rotate(float angle, float x, float y, float z) override final { NOT_IMPLEMENTED_FUNC(); }
		void Translate(float x, float y, float z) override final { NOT_IMPLEMENTED_FUNC(); }
		void Scale(float x, float y, float z) override final { NOT_IMPLEMENTED_FUNC(); }
		void ScaleXY(float x, float y) override final { NOT_IMPLEMENTED_FUNC(); }

		bool InFlashlightMode() const override final;

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

		void GetSceneFogColor(unsigned char* rgb) override { NOT_IMPLEMENTED_FUNC(); }
		MaterialFogMode_t GetSceneFogMode() override final;
		void SetPixelShaderFogParams(int reg) override final;
		MaterialFogMode_t GetCurrentFogType() const override { NOT_IMPLEMENTED_FUNC(); }
		int GetPixelFogCombo() override { NOT_IMPLEMENTED_FUNC_NOBREAK(); return 0; }

		void SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack) override final;
		void CommitPixelShaderLighting(int pshReg) override final;
		void GetDX9LightState(LightState_t* state) const override final;

		void GetWorldSpaceCameraPosition(float* pos) const override final;

		void SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale) override final;

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
}
