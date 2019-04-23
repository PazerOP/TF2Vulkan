#include "ShaderAPI.h"
#include "IShaderAPI2.h"

#include <cstdint>

using namespace TF2Vulkan;

namespace
{
	class ShaderAPI final : public IShaderAPI2
	{
	public:
		void SetViewports(int count, const ShaderViewport_t* viewports) override;
		int GetViewports(ShaderViewport_t* viewports, int max) const override;

		double CurrentTime() const override;

		void GetLightmapDimensions(int* w, int* h) override;

		MaterialFogMode_t GetSceneFogMode() override;

		void MatrixMode(MaterialMatrixMode_t mode) override;
		void PushMatrix() override;
		void PopMatrix() override;
		void LoadMatrix(float* m) override;
		void MultMatrix(float* m) override;
		void MultMatrixLocal(float* m) override;
		void GetMatrix(MaterialMatrixMode_t matrixMode, float* dst) override;
		void LoadIdentity() override;
		void LoadCameraToWorld() override;
		void Ortho(double left, double right, double bottom, double top, double zNear, double zFar) override;
		void PerspectiveX(double fovX, double aspect, double zNear, double zFar) override;
		void PerspectiveOffCenterX(double fovX, double aspect, double zNear, double zFar,
			double bottom, double top, double left, double right) override;
		void PickMatrix(int x, int y, int width, int height) override;
		void Rotate(float angle, float x, float y, float z) override;
		void Translate(float x, float y, float z) override;
		void Scale(float x, float y, float z) override;
		void ScaleXY(float x, float y) override;

		void Color3f(float r, float g, float b) override;
		void Color3fv(const float* rgb) override;
		void Color4f(float r, float g, float b, float a) override;
		void Color4fv(const float* rgba) override;

		void Color3ub(uint8_t r, uint8_t g, uint8_t b) override;
		void Color3ubv(const uint8_t* rgb) override;
		void Color4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override;
		void Color4ubv(const uint8_t* rgba) override;

		void SetVertexShaderConstant(int var, const float* vec, int numConst, bool force) override;
		void SetBooleanVertexShaderConstant(int var, const BOOL* vec, int numBools, bool force) override;
		void SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force) override;
		void SetPixelShaderConstant(int var, const float* vec, int numConst, bool force) override;
		void SetBooleanPixelShaderConstant(int var, const BOOL* vec, int numBools, bool force) override;
		void SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force) override;

		void SetDefaultState() override;

		void GetWorldSpaceCameraPosition(float* pos) const override;

		int GetCurrentNumBones() const override;
		int GetCurrentLightCombo() const override;

		MaterialFogMode_t GetCurrentFogType() const override;

		void SetTextureTransformDimension(TextureStage_t stage, int dimension, bool projected) override;
		void DisableTextureTransform(TextureStage_t stage) override;
		void SetBumpEnvMatrix(TextureStage_t stage, float m00, float m01, float m10, float m11) override;

		void SetVertexShaderIndex(int index) override;
		void SetPixelShaderIndex(int index) override;

		void GetBackBufferDimensions(int& width, int& height) const override;

		int GetMaxLights() const override;
		const LightDesc_t& GetLight(int lightNum) const override;

		void SetPixelShaderFogParams(int reg) override;

		void SetVertexShaderStateAmbientLightCube() override;
		void SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack) override;
		void CommitPixelShaderLighting(int pshReg) override;

		CMeshBuilder* GetVertexModifyBuilder() override;
		bool InFlashlightMode() const override;
		bool InEditorMode() const override;

		MorphFormat_t GetBoundMorphFormat() override;

		void ClearBuffersObeyStencil(bool clearColor, bool clearDepth) override;
		void ClearBuffersObeyStencilEx(bool clearColor, bool clearAlpha, bool clearDepth) override;
		void ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil, int rtWidth, int rtHeight) override;
		void ClearColor3ub(uint8_t r, uint8_t g, uint8_t b) override;
		void ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a) override;

		void BindVertexShader(VertexShaderHandle_t vtxShader) override;
		void BindGeometryShader(GeometryShaderHandle_t geoShader) override;
		void BindPixelShader(PixelShaderHandle_t pixShader) override;

		void SetRasterState(const ShaderRasterState_t& state) override;

		bool SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& info) override;

		void ChangeVideoMode(const ShaderDeviceInfo_t& info) override;

		StateSnapshot_t TakeSnapshot() override;

		void TexMinFilter(ShaderTexFilterMode_t mode) override;
		void TexMagFilter(ShaderTexFilterMode_t mode) override;
		void TexWrap(ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode) override;

		void CopyRenderTargetToTexture(ShaderAPITextureHandle_t texHandle) override;
		void CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t texHandle, int renderTargetID,
			const Rect_t* srcRect, const Rect_t* dstRect) override;
		void CopyTextureToRenderTargetEx(int renderTargetID, ShaderAPITextureHandle_t texHandle,
			const Rect_t* srcRect, const Rect_t* dstRect) override;
		void CopyRenderTargetToScratchTexture(ShaderAPITextureHandle_t srcRT,
			ShaderAPITextureHandle_t dstTex, const Rect_t* srcRect, const Rect_t* dstRect) override;

		void Bind(IMaterial* material) override;

		void FlushBufferedPrimitives() override;

		IMesh* GetDynamicMesh(IMaterial* material, int hwSkinBoneCount, bool buffered,
			IMesh* vertexOverride, IMesh* indexOverride) override;
		IMesh* GetDynamicMeshEx(IMaterial* material, VertexFormat_t vertexFormat, int hwSkinBoneCount,
			bool buffered, IMesh* vertexOverride, IMesh* indexOverride) override;

		bool IsTranslucent(StateSnapshot_t id) const override;
		bool IsAlphaTested(StateSnapshot_t id) const override;
		bool UsesVertexAndPixelShaders(StateSnapshot_t id) const override;
		bool IsDepthWriteEnabled(StateSnapshot_t id) const override;

		VertexFormat_t ComputeVertexFormat(int snapshotCount, StateSnapshot_t* ids) const override;
		VertexFormat_t ComputeVertexUsage(int snapshotCount, StateSnapshot_t* ids) const override;

		void BeginPass(StateSnapshot_t snapshot) override;
		void RenderPass(int passID, int passCount) override;
		void BeginFrame() override;
		void EndFrame() override;

		void FlushHardware() override;
		void ForceHardwareSync() override;

		void SetNumBoneWeights(int boneCount) override;

		void SetLight(int light, const LightDesc_t& desc) override;
		void SetLightingOrigin(Vector lightingOrigin) override;
		void SetAmbientLight(float r, float g, float b) override;
		void SetAmbientLightCube(Vector4D cube[6]) override;

		void ShadeMode(ShaderShadeMode_t mode) override;

		void CullMode(MaterialCullMode_t mode) override;

		void ForceDepthFuncEquals(bool enable) override;

		void OverrideDepthEnable(bool enable, bool depthEnable) override;

		void SetHeightClipZ(float z) override;
		void SetHeightClipMode(MaterialHeightClipMode_t mode) override;

		void SetClipPlane(int index, const float* plane) override;
		void EnableClipPlane(int index, bool enable) override;

		void SetSkinningMatrices() override;

		ImageFormat GetNearestSupportedFormat(ImageFormat fmt, bool filteringRequired) const override;
		ImageFormat GetNearestRenderTargetFormat(ImageFormat fmt) const override;

		bool DoRenderTargetsNeedSeparateDepthBuffer() const override;

		ShaderAPITextureHandle_t CreateTexture(int width, int height, int depth, ImageFormat dstImgFormat,
			int mipLevelCount, int copyCount, int flags, const char* dbgName, const char* texGroupName) override;
		void CreateTextures(ShaderAPITextureHandle_t* handles, int count, int width, int height, int depth,
			ImageFormat dstImgFormat, int mipLevelCount, int copyCount, int flags, const char* dbgName,
			const char* texGroupName) override;
		void DeleteTexture(ShaderAPITextureHandle_t tex) override;
		bool IsTexture(ShaderAPITextureHandle_t tex) override;
		bool IsTextureResident(ShaderAPITextureHandle_t tex) override;

		ShaderAPITextureHandle_t CreateDepthTexture(
			ImageFormat rtFormat, int width, int height, const char* dbgName, bool texture) override;

		void ModifyTexture(ShaderAPITextureHandle_t tex) override;

		void TexImage2D(int level, int cubeFaceID, ImageFormat dstFormat, int zOffset,
			int width, int height, ImageFormat srcFormat, bool srcIsTiled, void* imgData) override;
		void TexSubImage2D(int level, int cubeFaceID, int xOffset, int yOffset, int zOffset,
			int width, int height, ImageFormat srcFormat, int srcStride, bool srcIsTiled,
			void* imgData) override;

		void TexImageFromVTF(IVTFTexture* vtf, int ivtfFrame) override;

		bool TexLock(int level, int cubeFaceID, int xOffset, int yOffset,
			int width, int height, CPixelWriter& writer) override;
		void TexUnlock() override;

		void TexSetPriority(int priority) override;

		void BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t textureHandle) override;

		void SetRenderTarget(ShaderAPITextureHandle_t colTexHandle,
			ShaderAPITextureHandle_t depthTexHandle) override;
		void SetRenderTargetEx(int renderTargetID, ShaderAPITextureHandle_t colTex,
			ShaderAPITextureHandle_t depthTex) override;

		void ReadPixels(int x, int y, int width, int height, unsigned char* data,
			ImageFormat dstFormat) override;
		void ReadPixels(Rect_t* srcRect, Rect_t* dstRect, unsigned char* data,
			ImageFormat dstFormat, int dstStride) override;

		int SelectionMode(bool selectionMode) override;
		void SelectionBuffer(unsigned int* buf, int size) override;
		void ClearSelectionNames() override;
		void LoadSelectionName(int name) override;
		void PushSelectionName(int name) override;
		void PopSelectionName() override;

		void ClearSnapshots() override;

		void FogStart(float start) override;
		void FogEnd(float end) override;
		void SetFogZ(float fogZ) override;
		void SceneFogColor3ub(unsigned char r, unsigned char g, unsigned char b) override;
		void GetSceneFogColor(unsigned char* rgb) override;
		void SceneFogMode(MaterialFogMode_t mode) override;
		void GetFogDistances(float* start, float* end, float* fogZ) override;
		void FogMaxDensity(float maxDensity) override;

		bool CanDownloadTextures() const override;

		void ResetRenderState(bool fullReset) override;

		int GetCurrentDynamicVBSize() override;
		void DestroyVertexBuffers(bool exitingLevel) override;

		void EvictManagedResources() override;

		void SetAnisotropicLevel(int anisoLevel) override;

		void SyncToken(const char* token) override;

		void SetStandardVertexShaderConstants(float overbright) override;

		ShaderAPIOcclusionQuery_t CreateOcclusionQueryObject() override;
		void DestroyOcclusionQueryObject(ShaderAPIOcclusionQuery_t query) override;
		void BeginOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t query) override;
		void EndOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t query) override;
		int OcclusionQuery_GetNumPixelsRendered(ShaderAPIOcclusionQuery_t query, bool flush) override;

		const FlashlightState_t& GetFlashlightState(VMatrix& worldToTexture) const override;
		const FlashlightState_t& GetFlashlightStateEx(VMatrix& worldToTexture,
			ITexture** flashlightDepthTex) const override;
		void SetFlashlightState(const FlashlightState_t& state, const VMatrix& worldToTexture) override;
		void SetFlashlightStateEx(const FlashlightState_t& state, const VMatrix& worldToTexture,
			ITexture* flashlightDepthTex) override;

		void ClearVertexAndPixelShaderRefCounts() override;
		void PurgeUnusedVertexAndPixelShaders() override;

		void DXSupportLevelChanged() override;

		void EnableUserClipTransformOverride(bool enable) override;
		void UserClipTransform(const VMatrix& worldToView) override;

		MorphFormat_t ComputeMorphFormat(int snapshots, StateSnapshot_t* ids) const override;

		void HandleDeviceLost() override;

		void EnableLinearColorSpaceFrameBuffer(bool enable) override;

		void SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex) override;

		void SetFloatRenderingParameter(RenderParamFloat_t param, float value) override;
		void SetIntRenderingParameter(RenderParamInt_t param, int value) override;
		void SetVectorRenderingParameter(RenderParamVector_t param, const Vector& value) override;

		float GetFloatRenderingParameter(RenderParamFloat_t param) const override;
		int GetIntRenderingParameter(RenderParamInt_t param) const override;
		Vector GetVectorRenderingParameter(RenderParamVector_t param) const override;

		void SetFastClipPlane(const float* plane) override;
		void EnableFastClip(bool enable) override;

		void GetMaxToRender(IMesh* mesh, bool maxUntilFlush, int* maxVerts, int* maxIndices) override;

		int GetMaxVerticesToRender(IMaterial* material) override;
		int GetMaxIndicesToRender() override;

		void SetStencilEnable(bool enabled) override;
		void SetStencilFailOperation(StencilOperation_t op) override;
		void SetStencilZFailOperation(StencilOperation_t op) override;
		void SetStencilPassOperation(StencilOperation_t op) override;
		void SetStencilCompareFunction(StencilComparisonFunction_t fn) override;
		void SetStencilReferenceValue(int ref) override;
		void SetStencilTestMask(uint32 mask) override;
		void SetStencilWriteMask(uint32 mask) override;
		void ClearStencilBufferRectangle(int xmin, int ymin, int xmax, int ymax, int value) override;

		void DisableAllLocalLights() override;
		int CompareSnapshots(StateSnapshot_t lhs, StateSnapshot_t rhs) override;

		IMesh* GetFlexMesh() override;

		bool SupportsMSAAMode(int msaaMode) override;

		bool OwnGPUResources(bool enable) override;

		void BeginPIXEvent(unsigned long color, const char* szName) override;
		void EndPIXEvent() override;
		void SetPIXMarker(unsigned long color, const char* szName) override;

		void EnableAlphaToCoverage() override;
		void DisableAlphaToCoverage() override;

		void ComputeVertexDescription(unsigned char* buffer, VertexFormat_t fmt, MeshDesc_t& desc) const override;

		bool SupportsShadowDepthTextures() override;

		void SetDisallowAccess(bool disallowed) override;
		void EnableShaderShaderMutex(bool enabled) override;
		void ShaderLock() override;
		void ShaderUnlock() override;

		ImageFormat GetShadowDepthTextureFormat() override;

		bool SupportsFetch4() override;
		void SetShadowDepthBiasFactors(float slopeScaleDepthBias, float shadowDepthBias) override;

		void BindVertexBuffer(int streamID, IVertexBuffer* vtxBuf, int byteOffset, int firstVertex, int vtxCount,
			VertexFormat_t fmt, int repetitions) override;
		void BindIndexBuffer(IIndexBuffer* indexBuf, int offsetInBytes) override;
		void Draw(MaterialPrimitiveType_t type, int firstIndex, int indexCount) override;

		void PerformFullScreenStencilOperation() override;

		void SetScissorRect(const int left, const int top, const int right, const int bottom,
			const bool enableScissor) override;

		bool SupportsCSAAMode(int numSamples, int qualityLevel) override;

		void InvalidateDelayedShaderConstants() override;

		float GammaToLinear_HardwareSpecific(float gamma) const override;
		float LinearToGamma_HardwareSpecific(float linear) const override;

		void SetLinearToGammaConversionTextures(ShaderAPITextureHandle_t srgbWriteEnabledTex,
			ShaderAPITextureHandle_t identityTex) override;

		ImageFormat GetNullTextureFormat() override;

		void BindVertexTexture(VertexTextureSampler_t sampler, ShaderAPITextureHandle_t tex) override;

		void EnableHWMorphing(bool enabled) override;

		void SetFlexWeights(int firstWeight, int count, const MorphWeight_t* weights) override;

		void AcquireThreadOwnership() override;
		void ReleaseThreadOwnership() override;

		bool SupportsNormalMapCompression() const override;

		void EnableBuffer2FramesAhead(bool enable) override;

		void SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale) override;

		void PrintfVA(char* fmt, va_list vargs) override;
		void Printf(PRINTF_FORMAT_STRING const char* fmt, ...) override;
		float Knob(char* knobName, float* setValue) override;

		void OverrideAlphaWriteEnable(bool enable, bool alphaWriteEnable) override;
		void OverrideColorWriteEnable(bool overrideEnable, bool colorWriteEnable) override;

		void LockRect(void** outBits, int* outPitch, ShaderAPITextureHandle_t tex, int mipLevel,
			int x, int y, int w, int h, bool write, bool read) override;
		void UnlockRect(ShaderAPITextureHandle_t tex, int mipLevel) override;

		void BindStandardTexture(Sampler_t sampler, StandardTextureId_t id) override;
		void BindStandardVertexTexture(VertexTextureSampler_t sampler, StandardTextureId_t id) override;

		ITexture* GetRenderTargetEx(int renderTargetID) override;

		void SetToneMappingScaleLinear(const Vector& scale) override;
		const Vector& GetToneMappingScaleLinear() const override;
		float GetLightMapScaleFactor() const override;

		void LoadBoneMatrix(int boneIndex, const float* m) override;

		void GetDXLevelDefaults(uint& dxLevelMax, uint& dxLevelRecommended) override;

		float GetAmbientLightCubeLuminance() override;

		void GetDX9LightState(LightState_t* state) const override;
		int GetPixelFogCombo() override;

		bool IsHWMorphingEnabled() const override;

		void GetStandardTextureDimensions(int* width, int* height, StandardTextureId_t id) override;

		bool ShouldWriteDepthToDestAlpha() const override;

		void PushDeformation(const DeformationBase_t* deformation) override;
		void PopDeformation() override;
		int GetNumActiveDeformations() const override;

		int GetPackedDeformationInformation(int maxOfUnderstoodDeformations, float* constantValuesOut,
			int bufSize, int maxDeformations, int* numDefsOut) const override;

		void MarkUnusedVertexFields(unsigned int flags, int texCoordCount, bool* unusedTexCoords) override;

		void ExecuteCommandBuffer(uint8* cmdBuf) override;

		void SetStandardTextureHandle(StandardTextureId_t id, ShaderAPITextureHandle_t tex) override;
		void GetCurrentColorCorrection(ShaderColorCorrectionInfo_t* info) override;

		void SetPSNearAndFarZ(int psRegister) override;
	};
}

static ShaderAPI s_ShaderAPI;
IShaderAPI* TF2Vulkan::GetShaderAPI() { return &s_ShaderAPI; }
