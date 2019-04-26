#include "FormatConversion.h"
#include "ShaderAPI.h"
#include "ShaderDevice.h"
#include "ShadowStateManager.h"
#include "TF2Vulkan/TextureData.h"
#include "VulkanMesh.h"

#include <TF2Vulkan/Util/DirtyVar.h>
#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/ImageManip.h>
#include <TF2Vulkan/Util/std_utility.h>

#include <materialsystem/imesh.h>

#include "vk_mem_alloc.h"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>

using namespace TF2Vulkan;

namespace
{
	union SamplerSettingsDynamic
	{
		constexpr SamplerSettingsDynamic() :
			m_MinFilter(SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR),
			m_MagFilter(SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR),

			m_WrapS(SHADER_TEXWRAPMODE_REPEAT),
			m_WrapT(SHADER_TEXWRAPMODE_REPEAT),
			m_WrapU(SHADER_TEXWRAPMODE_REPEAT)
		{
		}

		constexpr bool operator==(const SamplerSettingsDynamic& s) const
		{
			return m_BackingValue == s.m_BackingValue;
		}

		struct
		{
			ShaderTexFilterMode_t m_MinFilter : 3;
			ShaderTexFilterMode_t m_MagFilter : 3;

			ShaderTexWrapMode_t m_WrapS : 2;
			ShaderTexWrapMode_t m_WrapT : 2;
			ShaderTexWrapMode_t m_WrapU : 2;
		};
		int m_BackingValue;
	};
	static_assert(sizeof(SamplerSettingsDynamic::m_BackingValue) == sizeof(SamplerSettingsDynamic));
}

template<> struct ::std::hash<SamplerSettingsDynamic>
{
	size_t operator()(const SamplerSettingsDynamic& s) const
	{
		return Util::hash_value(s.m_BackingValue);
	}
};

namespace
{
	class ShaderAPI final : public IShaderAPIInternal
	{
	public:
		void SetViewports(int count, const ShaderViewport_t* viewports) override;
		int GetViewports(ShaderViewport_t* viewports, int max) const override;

		double CurrentTime() const override;

		void GetLightmapDimensions(int* w, int* h) override;

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

		void SetTextureTransformDimension(TextureStage_t stage, int dimension, bool projected) override;
		void DisableTextureTransform(TextureStage_t stage) override;
		void SetBumpEnvMatrix(TextureStage_t stage, float m00, float m01, float m10, float m11) override;

		void SetVertexShaderIndex(int index) override;
		void SetPixelShaderIndex(int index) override;

		void GetBackBufferDimensions(int& width, int& height) const override;

		int GetMaxLights() const override;
		const LightDesc_t& GetLight(int lightNum) const override;

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

		void TexMinFilter(ShaderAPITextureHandle_t tex, ShaderTexFilterMode_t mode) override;
		void TexMagFilter(ShaderAPITextureHandle_t tex, ShaderTexFilterMode_t mode) override;
		void TexWrap(ShaderAPITextureHandle_t tex, ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode) override;

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

		void TexImageFromVTF(ShaderAPITextureHandle_t texHandle, IVTFTexture* vtf, int ivtfFrame) override;

		bool UpdateTexture(ShaderAPITextureHandle_t texHandle, const TextureData* data,
			size_t count) override;

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
		MaterialFogMode_t GetSceneFogMode() override;
		void SetPixelShaderFogParams(int reg) override;
		MaterialFogMode_t GetCurrentFogType() const override;

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

		void TexLodClamp(int something) override;
		void TexLodBias(float bias) override;

		void CopyTextureToTexture(int something1, int something2) override { NOT_IMPLEMENTED_FUNC(); }

	private:
		std::recursive_mutex m_ShaderLock;

		std::atomic<ShaderAPITextureHandle_t> m_NextTextureHandle = 1;

		struct ShaderSampler
		{
		};
		std::unordered_map<SamplerSettingsDynamic, ShaderSampler> m_ShaderSamplers;

		struct DynamicState
		{
			IMaterial* m_BoundMaterial = nullptr;
			int m_BoneCount = 0;

		} m_DynamicState;
		bool m_DynamicStateDirty = true;

		struct ShaderTexture
		{
			std::string m_DebugName;
			vk::ImageCreateInfo m_CreateInfo;
			vma::AllocatedImage m_Image;

			SamplerSettingsDynamic m_SamplerSettings;
		};
		std::unordered_map<ShaderAPITextureHandle_t, ShaderTexture> m_Textures;

		std::unordered_map<VertexFormat, VulkanMesh> m_DynamicMeshes;
	};
}

static ShaderAPI s_ShaderAPI;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShaderAPI, IShaderAPI, SHADERAPI_INTERFACE_VERSION, s_ShaderAPI);
IShaderAPI* GetShaderAPI() { return &s_ShaderAPI; }

void ShaderAPI::SetViewports(int count, const ShaderViewport_t* viewports)
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetViewports(ShaderViewport_t* viewports, int max) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

double ShaderAPI::CurrentTime() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0;
}

void ShaderAPI::GetLightmapDimensions(int* w, int* h)
{
	NOT_IMPLEMENTED_FUNC();
}

MaterialFogMode_t ShaderAPI::GetSceneFogMode()
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialFogMode_t();
}

void ShaderAPI::MatrixMode(MaterialMatrixMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PushMatrix()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PopMatrix()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::LoadMatrix(float* m)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::MultMatrix(float* m)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::MultMatrixLocal(float* m)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetMatrix(MaterialMatrixMode_t matrixMode, float* dst)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::LoadIdentity()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::LoadCameraToWorld()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Ortho(double left, double right, double bottom, double top, double zNear, double zFar)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PerspectiveX(double fovX, double aspect, double zNear, double zFar)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PerspectiveOffCenterX(double fovX, double aspect, double zNear, double zFar, double bottom, double top, double left, double right)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PickMatrix(int x, int y, int width, int height)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Rotate(float angle, float x, float y, float z)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Translate(float x, float y, float z)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Scale(float x, float y, float z)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ScaleXY(float x, float y)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color3f(float r, float g, float b)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color3fv(const float* rgb)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color4f(float r, float g, float b, float a)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color4fv(const float* rgba)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color3ub(uint8_t r, uint8_t g, uint8_t b)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color3ubv(const uint8_t* rgb)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Color4ubv(const uint8_t* rgba)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetVertexShaderConstant(int var, const float* vec, int numConst, bool force)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetBooleanVertexShaderConstant(int var, const BOOL* vec, int numBools, bool force)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetIntegerVertexShaderConstant(int var, const int* vec, int numIntVecs, bool force)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetPixelShaderConstant(int var, const float* vec, int numConst, bool force)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetBooleanPixelShaderConstant(int var, const BOOL* vec, int numBools, bool force)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetIntegerPixelShaderConstant(int var, const int* vec, int numIntVecs, bool force)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetDefaultState()
{
	LOG_FUNC();
	m_DynamicState = {};
	m_DynamicStateDirty = true;
}

void ShaderAPI::GetWorldSpaceCameraPosition(float* pos) const
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetCurrentNumBones() const
{
	LOG_FUNC();
	return m_DynamicState.m_BoneCount;
}

int ShaderAPI::GetCurrentLightCombo() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

MaterialFogMode_t ShaderAPI::GetCurrentFogType() const
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialFogMode_t();
}

void ShaderAPI::SetTextureTransformDimension(TextureStage_t stage, int dimension, bool projected)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::DisableTextureTransform(TextureStage_t stage)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetBumpEnvMatrix(TextureStage_t stage, float m00, float m01, float m10, float m11)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetVertexShaderIndex(int index)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetPixelShaderIndex(int index)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetBackBufferDimensions(int& width, int& height) const
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetMaxLights() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

const LightDesc_t& ShaderAPI::GetLight(int lightNum) const
{
	NOT_IMPLEMENTED_FUNC();
	// TODO: insert return statement here
	static const LightDesc_t s_TempLightDesc{};
	return s_TempLightDesc;
}

void ShaderAPI::SetPixelShaderFogParams(int reg)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetVertexShaderStateAmbientLightCube()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetPixelShaderStateAmbientLightCube(int pshReg, bool forceToBlack)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::CommitPixelShaderLighting(int pshReg)
{
	NOT_IMPLEMENTED_FUNC();
}

CMeshBuilder* ShaderAPI::GetVertexModifyBuilder()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool ShaderAPI::InFlashlightMode() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderAPI::InEditorMode() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

MorphFormat_t ShaderAPI::GetBoundMorphFormat()
{
	NOT_IMPLEMENTED_FUNC();
	return MorphFormat_t();
}

void ShaderAPI::ClearBuffersObeyStencil(bool clearColor, bool clearDepth)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearBuffersObeyStencilEx(bool clearColor, bool clearAlpha, bool clearDepth)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil, int rtWidth, int rtHeight)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearColor3ub(uint8_t r, uint8_t g, uint8_t b)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindVertexShader(VertexShaderHandle_t vtxShader)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindGeometryShader(GeometryShaderHandle_t geoShader)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindPixelShader(PixelShaderHandle_t pixShader)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetRasterState(const ShaderRasterState_t& state)
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderAPI::SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& info)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::ChangeVideoMode(const ShaderDeviceInfo_t& info)
{
	NOT_IMPLEMENTED_FUNC();
}

StateSnapshot_t ShaderAPI::TakeSnapshot()
{
	LOG_FUNC();

	auto snapshot = g_ShadowStateManager.TakeSnapshot();
	return Util::SafeConvert<StateSnapshot_t>(snapshot);
}

void ShaderAPI::TexMinFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);
	tex.m_SamplerSettings.m_MinFilter = mode;
}

void ShaderAPI::TexMagFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);
	tex.m_SamplerSettings.m_MagFilter = mode;
}

void ShaderAPI::TexWrap(ShaderAPITextureHandle_t texHandle, ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);

	auto& ss = tex.m_SamplerSettings;
	switch (coord)
	{
	case SHADER_TEXCOORD_S:
		ss.m_WrapS = wrapMode;
		break;
	case SHADER_TEXCOORD_T:
		ss.m_WrapT = wrapMode;
		break;
	case SHADER_TEXCOORD_U:
		ss.m_WrapU = wrapMode;
		break;

	default:
		Warning(TF2VULKAN_PREFIX "Invalid wrapMode %i\n", int(wrapMode));
	}
}

void ShaderAPI::CopyRenderTargetToTexture(ShaderAPITextureHandle_t texHandle)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t texHandle, int renderTargetID, const Rect_t* srcRect, const Rect_t* dstRect)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::CopyTextureToRenderTargetEx(int renderTargetID, ShaderAPITextureHandle_t texHandle, const Rect_t* srcRect, const Rect_t* dstRect)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::CopyRenderTargetToScratchTexture(ShaderAPITextureHandle_t srcRT, ShaderAPITextureHandle_t dstTex, const Rect_t* srcRect, const Rect_t* dstRect)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Bind(IMaterial* material)
{
	LOG_FUNC();
	Util::SetDirtyVar(m_DynamicState.m_BoundMaterial, material, m_DynamicStateDirty);
}

void ShaderAPI::FlushBufferedPrimitives()
{
	LOG_FUNC();
	// TODO
	//NOT_IMPLEMENTED_FUNC();
}

IMesh* ShaderAPI::GetDynamicMesh(IMaterial* material, int hwSkinBoneCount,
	bool buffered, IMesh* vertexOverride, IMesh* indexOverride)
{
	LOG_FUNC();
	return GetDynamicMeshEx(material, material->GetVertexFormat(), hwSkinBoneCount, buffered, vertexOverride, indexOverride);
	const VertexFormat fmt(material->GetVertexFormat());
}

IMesh* ShaderAPI::GetDynamicMeshEx(IMaterial* material, VertexFormat_t vertexFormat,
	int hwSkinBoneCount, bool buffered, IMesh* vertexOverride, IMesh* indexOverride)
{
	LOG_FUNC();

	const VertexFormat fmt(vertexFormat);

	if (auto found = m_DynamicMeshes.find(fmt); found != m_DynamicMeshes.end())
		return &found->second;

	return &m_DynamicMeshes.emplace(fmt, VulkanMesh(fmt)).first->second;
}

bool ShaderAPI::IsTranslucent(StateSnapshot_t id) const
{
	LOG_FUNC();
	return g_ShadowStateManager.IsTranslucent(id);
}

bool ShaderAPI::IsAlphaTested(StateSnapshot_t id) const
{
	LOG_FUNC();
	return g_ShadowStateManager.IsAlphaTested(id);
}

bool ShaderAPI::UsesVertexAndPixelShaders(StateSnapshot_t id) const
{
	LOG_FUNC();
	return g_ShadowStateManager.UsesVertexAndPixelShaders(id);
}

bool ShaderAPI::IsDepthWriteEnabled(StateSnapshot_t id) const
{
	LOG_FUNC();
	return g_ShadowStateManager.IsDepthWriteEnabled(id);
}

VertexFormat_t ShaderAPI::ComputeVertexFormat(int snapshotCount, StateSnapshot_t* ids) const
{
	LOG_FUNC();

	assert(snapshotCount > 0);
	assert(ids);
	if (snapshotCount <= 0 || !ids)
		return VERTEX_FORMAT_UNKNOWN;

	const auto& vtxFmt0 = g_ShadowStateManager.GetState(ids[0]).m_VertexShader.m_VertexFormat;
	VertexCompressionType_t compression = CompressionType(vtxFmt0);
	uint_fast8_t userDataSize = UserDataSize(vtxFmt0);
	uint_fast8_t boneCount = NumBoneWeights(vtxFmt0);
	VertexFormatFlags flags = VertexFormatFlags(VertexFlags(vtxFmt0));
	uint_fast8_t texCoordSize[VERTEX_MAX_TEXTURE_COORDINATES];
	for (size_t i = 0; i < std::size(texCoordSize); i++)
		texCoordSize[i] = TexCoordSize(i, vtxFmt0);

	for (int i = 1; i < snapshotCount; i++)
	{
		const auto& fmt = g_ShadowStateManager.GetState(*ids).m_VertexShader.m_VertexFormat;

		if (auto thisComp = CompressionType(fmt); thisComp != compression)
		{
			Warning(TF2VULKAN_PREFIX "Encountered a material with two passes that specify different vertex compression types!\n");
			assert(false);
			compression = VERTEX_COMPRESSION_NONE;
		}

		if (auto thisBoneCount = NumBoneWeights(fmt); thisBoneCount != boneCount)
		{
			Warning(TF2VULKAN_PREFIX "Encountered a material with two passes that use different numbers of bones!\n");
			assert(false);
		}

		if (auto thisUserDataSize = UserDataSize(fmt); thisUserDataSize != userDataSize)
		{
			Warning(TF2VULKAN_PREFIX "Encountered a material with two passes that use different user data sizes!\n");
			assert(false);
		}

		for (size_t t = 0; t < std::size(texCoordSize); t++)
		{
			auto& oldTCD = texCoordSize[t];
			if (auto thisTexCoordDim = TexCoordSize(t, fmt); thisTexCoordDim != oldTCD)
			{
				if (oldTCD != 0)
					Warning(TF2VULKAN_PREFIX "Encountered a material with two passes that use different texture coord sizes!\n");
				//assert(false);

				if (thisTexCoordDim > oldTCD)
					oldTCD = thisTexCoordDim;
			}
		}
	}

	// Untested;
	assert(boneCount == 0);
	assert(userDataSize == 0);

	VertexFormat_t retVal =
		VertexFormat_t(flags) |
		VERTEX_BONEWEIGHT(boneCount) |
		VERTEX_USERDATA_SIZE(userDataSize);

	for (size_t i = 0; i < std::size(texCoordSize); i++)
		retVal |= VERTEX_TEXCOORD_SIZE(i, texCoordSize[i]);

	return retVal;
}

VertexFormat_t ShaderAPI::ComputeVertexUsage(int snapshotCount, StateSnapshot_t* ids) const
{
	LOG_FUNC();
	return ComputeVertexFormat(snapshotCount, ids);
}

void ShaderAPI::BeginPass(StateSnapshot_t snapshot)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::RenderPass(int passID, int passCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BeginFrame()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EndFrame()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::FlushHardware()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ForceHardwareSync()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetNumBoneWeights(int boneCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetLight(int light, const LightDesc_t& desc)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetLightingOrigin(Vector lightingOrigin)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetAmbientLight(float r, float g, float b)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetAmbientLightCube(Vector4D cube[6])
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ShadeMode(ShaderShadeMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::CullMode(MaterialCullMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ForceDepthFuncEquals(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::OverrideDepthEnable(bool enable, bool depthEnable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetHeightClipZ(float z)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetHeightClipMode(MaterialHeightClipMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetClipPlane(int index, const float* plane)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EnableClipPlane(int index, bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetSkinningMatrices()
{
	NOT_IMPLEMENTED_FUNC();
}

ImageFormat ShaderAPI::GetNearestSupportedFormat(ImageFormat fmt, bool filteringRequired) const
{
	LOG_FUNC();
	return PromoteToHardware(fmt, FormatUsage::ImmutableTexture, filteringRequired);
}

ImageFormat ShaderAPI::GetNearestRenderTargetFormat(ImageFormat fmt) const
{
	LOG_FUNC();
	return PromoteToHardware(fmt, FormatUsage::RenderTarget, true);
}

bool ShaderAPI::DoRenderTargetsNeedSeparateDepthBuffer() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

ShaderAPITextureHandle_t ShaderAPI::CreateTexture(int width, int height, int depth,
	ImageFormat dstImgFormat, int mipLevelCount, int copyCount, int flags,
	const char* dbgName, const char* texGroupName)
{
	LOG_FUNC();
	ENSURE(width > 0);
	ENSURE(height > 0);
	ENSURE(depth > 0);
	ENSURE(mipLevelCount > 0);

	vk::ImageCreateInfo createInfo;

	if (height == 1 && depth == 1)
		createInfo.imageType = vk::ImageType::e1D;
	else if (depth == 1)
		createInfo.imageType = vk::ImageType::e2D;
	else
		createInfo.imageType = vk::ImageType::e3D;

	createInfo.extent.width = uint32_t(width);
	createInfo.extent.height = uint32_t(height);
	createInfo.extent.depth = uint32_t(depth);
	createInfo.arrayLayers = 1; // No support for texture arrays in stock valve materialsystem
	createInfo.mipLevels = uint32_t(mipLevelCount);
	createInfo.format = ConvertImageFormat(dstImgFormat);
	createInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;

	auto& vulkanDevice = g_ShaderDevice.GetVulkanDevice();

	auto created = vulkanDevice.createImageUnique(createInfo);
	if (!created)
	{
		Warning("[TF2Vulkan] %s(): Failed to create texture \"%s\"\n", __FUNCTION__, dbgName ? dbgName : "<null>");
		return INVALID_SHADERAPI_TEXTURE_HANDLE;
	}

	vma::AllocationCreateInfo allocCreateInfo;
	allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	auto createdImg = g_ShaderDevice.GetVulkanAllocator().createImageUnique(createInfo, allocCreateInfo);

	const auto handle = m_NextTextureHandle++;
	m_Textures[handle] = { dbgName, std::move(createInfo), std::move(createdImg) };
	return handle;
}

void ShaderAPI::CreateTextures(ShaderAPITextureHandle_t* handles, int count, int width, int height, int depth, ImageFormat dstImgFormat, int mipLevelCount, int copyCount, int flags, const char* dbgName, const char* texGroupName)
{
	LOG_FUNC();

	for (int i = 0; i < count; i++)
	{
		handles[i] = CreateTexture(width, height, depth, dstImgFormat,
			mipLevelCount, copyCount, flags, dbgName, texGroupName);
	}
}

void ShaderAPI::DeleteTexture(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC();

#ifdef _DEBUG
	[[maybe_unused]] auto realTex = m_Textures.find(tex);
#endif

	m_Textures.erase(tex);
}

bool ShaderAPI::IsTexture(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC();

	bool found = m_Textures.find(tex) != m_Textures.end();
	assert(found);
	return found;
}

bool ShaderAPI::IsTextureResident(ShaderAPITextureHandle_t tex)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

ShaderAPITextureHandle_t ShaderAPI::CreateDepthTexture(ImageFormat rtFormat, int width, int height, const char* dbgName, bool texture)
{
	NOT_IMPLEMENTED_FUNC();
	return ShaderAPITextureHandle_t();
}

void ShaderAPI::TexImageFromVTF(ShaderAPITextureHandle_t texHandle, IVTFTexture* vtf, int frameIndex)
{
	LOG_FUNC();

	const auto mipCount = vtf->MipCount();
	ENSURE(mipCount > 0);

	const auto faceCount = vtf->FaceCount();
	ENSURE(faceCount > 0);

	const auto arraySize = vtf->MipCount() * vtf->FaceCount();
	auto* texDatas = (TextureData*)stackalloc(arraySize * sizeof(TextureData));

	for (int mip = 0; mip < mipCount; mip++)
	{
		int width, height, depth;
		vtf->ComputeMipLevelDimensions(mip, &width, &height, &depth);

		const int mipSize = vtf->ComputeMipSize(mip);
		const int stride = vtf->RowSizeInBytes(mip);

		for (int face = 0; face < faceCount; face++)
		{
			TextureData& texData = texDatas[mip * faceCount + face];
			texData = {};
			texData.m_Format = vtf->Format();

			Util::SafeConvert(width, texData.m_Width);
			Util::SafeConvert(height, texData.m_Height);
			Util::SafeConvert(depth, texData.m_Depth);
			texData.m_Data = vtf->ImageData(frameIndex, face, mip);
			Util::SafeConvert(mipSize, texData.m_DataLength);
			Util::SafeConvert(stride, texData.m_Stride);

			Util::SafeConvert(mip, texData.m_MipLevel);
			texData.m_CubeFace = CubeMapFaceIndex_t(face);

			texData.Validate();
		}
	}

	UpdateTexture(texHandle, texDatas, arraySize);
}

static void TransitionImageLayout(const vk::Image& image, const vk::Format& format,
	const vk::ImageLayout& oldLayout, const vk::ImageLayout& newLayout,
	vk::CommandBuffer cmdBuf, uint32_t mipLevel)
{
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;

	auto& srr = barrier.subresourceRange;
	srr.aspectMask = vk::ImageAspectFlagBits::eColor;
	srr.baseMipLevel = mipLevel;
	srr.levelCount = 1;
	srr.baseArrayLayer = 0;
	srr.layerCount = 1;

	vk::PipelineStageFlags srcStageMask;
	vk::PipelineStageFlags dstStageMask;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		srcStageMask = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStageMask = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStageMask = vk::PipelineStageFlagBits::eTransfer;
		dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw VulkanException("Unsupported layout transition", EXCEPTION_DATA());
	}

	cmdBuf.pipelineBarrier(
		srcStageMask,
		dstStageMask,
		vk::DependencyFlags{}, {}, {}, barrier);
}

static void CopyBufferToImage(const vk::Buffer& buf, const vk::Image& img, uint32_t width, uint32_t height,
	vk::CommandBuffer cmdBuf)
{
	vk::BufferImageCopy copy;

	copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copy.imageSubresource.layerCount = 1;
	copy.imageExtent = { width, height, 1 };

	cmdBuf.copyBufferToImage(buf, img, vk::ImageLayout::eTransferDstOptimal, copy);
}

bool ShaderAPI::UpdateTexture(ShaderAPITextureHandle_t texHandle, const TextureData* data, size_t count)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);

	auto& device = g_ShaderDevice.GetVulkanDevice();
	auto& alloc = g_ShaderDevice.GetVulkanAllocator();
	auto& queue = g_ShaderDevice.GetGraphicsQueue();
	//const auto memReqs = device.getImageMemoryRequirements(tex.m_Image.m_Image.get());

	// Prepare the staging buffer
	vma::AllocatedBuffer stagingBuf;
	{
		size_t totalSize = 0;
		for (size_t i = 0; i < count; i++)
		{
			totalSize += data[i].m_DataLength;
		}

		// Allocate staging buffer
		{
			vk::BufferCreateInfo stagingBufCreateInfo;
			stagingBufCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
			stagingBufCreateInfo.size = totalSize;

			vma::AllocationCreateInfo transferBufAllocInfo;
			transferBufAllocInfo.requiredFlags = VkMemoryPropertyFlags(
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			stagingBuf = alloc.createBufferUnique(stagingBufCreateInfo, transferBufAllocInfo);
		}

		// Copy the data into the staging buffer
		size_t currentOffset = 0;
		for (size_t i = 0; i < count; i++)
		{
			stagingBuf.m_Allocation.map().Write(data[i].m_Data, data[i].m_DataLength, currentOffset);
			currentOffset += data[i].m_DataLength;
		}
	}

	// Copy staging buffer into destination texture
	{
		vk::CommandBufferAllocateInfo cmdAllocInfo;
		cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
		cmdAllocInfo.commandPool = queue.GetCmdPool();
		cmdAllocInfo.commandBufferCount = 1;

		auto cmdBuffer = std::move(device.allocateCommandBuffersUnique(cmdAllocInfo).front());
		if (!cmdBuffer)
			throw VulkanException("Failed to create temporary command buffer", EXCEPTION_DATA());

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		cmdBuffer->begin(beginInfo);

		TransitionImageLayout(tex.m_Image.m_Image, tex.m_CreateInfo.format,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, cmdBuffer.get(), 0);

		CopyBufferToImage(stagingBuf.m_Buffer, tex.m_Image.m_Image, tex.m_CreateInfo.extent.width,
			tex.m_CreateInfo.extent.height, cmdBuffer.get());

		TransitionImageLayout(tex.m_Image.m_Image, tex.m_CreateInfo.format,
			vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, cmdBuffer.get(), 0);

		cmdBuffer->end();

		// Submit the command buffer and wait for it to finish
		{
			vk::SubmitInfo submit;
			submit.commandBufferCount = 1;
			submit.pCommandBuffers = &cmdBuffer.get();

			auto& vQueue = queue.GetQueue();
			vQueue.submit(submit, nullptr);
			vQueue.waitIdle();
		}
	}

	return true;
}

bool ShaderAPI::TexLock(int level, int cubeFaceID, int xOffset, int yOffset, int width, int height, CPixelWriter& writer)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::TexUnlock()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::TexSetPriority(int priority)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindTexture(Sampler_t sampler, ShaderAPITextureHandle_t textureHandle)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetRenderTarget(ShaderAPITextureHandle_t colTexHandle, ShaderAPITextureHandle_t depthTexHandle)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetRenderTargetEx(int renderTargetID, ShaderAPITextureHandle_t colTex, ShaderAPITextureHandle_t depthTex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ReadPixels(int x, int y, int width, int height, unsigned char* data, ImageFormat dstFormat)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ReadPixels(Rect_t* srcRect, Rect_t* dstRect, unsigned char* data, ImageFormat dstFormat, int dstStride)
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::SelectionMode(bool selectionMode)
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderAPI::SelectionBuffer(unsigned int* buf, int size)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearSelectionNames()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::LoadSelectionName(int name)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PushSelectionName(int name)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PopSelectionName()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearSnapshots()
{
	LOG_FUNC();
	// We never want to clear "snapshots" (graphics pipelines) on vulkan

	FlushBufferedPrimitives(); // This is a side effect of this function
}

void ShaderAPI::FogStart(float start)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::FogEnd(float end)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetFogZ(float fogZ)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SceneFogColor3ub(unsigned char r, unsigned char g, unsigned char b)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetSceneFogColor(unsigned char* rgb)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SceneFogMode(MaterialFogMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetFogDistances(float* start, float* end, float* fogZ)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::FogMaxDensity(float maxDensity)
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderAPI::CanDownloadTextures() const
{
	LOG_FUNC();
	return true;
}

void ShaderAPI::ResetRenderState(bool fullReset)
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetCurrentDynamicVBSize()
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderAPI::DestroyVertexBuffers(bool exitingLevel)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EvictManagedResources()
{
	LOG_FUNC();
	// Nothing to do here (for now...)
}

void ShaderAPI::SetAnisotropicLevel(int anisoLevel)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SyncToken(const char* token)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStandardVertexShaderConstants(float overbright)
{
	NOT_IMPLEMENTED_FUNC();
}

ShaderAPIOcclusionQuery_t ShaderAPI::CreateOcclusionQueryObject()
{
	NOT_IMPLEMENTED_FUNC();
	return ShaderAPIOcclusionQuery_t();
}

void ShaderAPI::DestroyOcclusionQueryObject(ShaderAPIOcclusionQuery_t query)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BeginOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t query)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EndOcclusionQueryDrawing(ShaderAPIOcclusionQuery_t query)
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::OcclusionQuery_GetNumPixelsRendered(ShaderAPIOcclusionQuery_t query, bool flush)
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

const FlashlightState_t& ShaderAPI::GetFlashlightState(VMatrix& worldToTexture) const
{
	NOT_IMPLEMENTED_FUNC();
	// TODO: insert return statement here
	static const FlashlightState_t s_TempFlashlightState{};
	return s_TempFlashlightState;
}

const FlashlightState_t& ShaderAPI::GetFlashlightStateEx(VMatrix& worldToTexture, ITexture** flashlightDepthTex) const
{
	NOT_IMPLEMENTED_FUNC();
	// TODO: insert return statement here
	static const FlashlightState_t s_TempFlashlightState{};
	return s_TempFlashlightState;
}

void ShaderAPI::SetFlashlightState(const FlashlightState_t& state, const VMatrix& worldToTexture)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetFlashlightStateEx(const FlashlightState_t& state, const VMatrix& worldToTexture, ITexture* flashlightDepthTex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearVertexAndPixelShaderRefCounts()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PurgeUnusedVertexAndPixelShaders()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::DXSupportLevelChanged()
{
	LOG_FUNC();
	// We don't care
}

void ShaderAPI::EnableUserClipTransformOverride(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::UserClipTransform(const VMatrix& worldToView)
{
	NOT_IMPLEMENTED_FUNC();
}

MorphFormat_t ShaderAPI::ComputeMorphFormat(int snapshots, StateSnapshot_t* ids) const
{
	MorphFormat_t fmt = {};

	for (int i = 0; i < snapshots; i++)
		fmt |= g_ShadowStateManager.GetState(ids[i]).m_VertexShader.m_MorphFormat;

	return fmt;
}

void ShaderAPI::HandleDeviceLost()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EnableLinearColorSpaceFrameBuffer(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetFullScreenTextureHandle(ShaderAPITextureHandle_t tex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetFloatRenderingParameter(RenderParamFloat_t param, float value)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetIntRenderingParameter(RenderParamInt_t param, int value)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetVectorRenderingParameter(RenderParamVector_t param, const Vector& value)
{
	NOT_IMPLEMENTED_FUNC();
}

float ShaderAPI::GetFloatRenderingParameter(RenderParamFloat_t param) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

int ShaderAPI::GetIntRenderingParameter(RenderParamInt_t param) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

Vector ShaderAPI::GetVectorRenderingParameter(RenderParamVector_t param) const
{
	NOT_IMPLEMENTED_FUNC();
	return Vector();
}

void ShaderAPI::SetFastClipPlane(const float* plane)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EnableFastClip(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetMaxToRender(IMesh* mesh, bool maxUntilFlush, int* maxVerts, int* maxIndices)
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetMaxVerticesToRender(IMaterial* material)
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int ShaderAPI::GetMaxIndicesToRender()
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderAPI::SetStencilEnable(bool enabled)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilFailOperation(StencilOperation_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilZFailOperation(StencilOperation_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilPassOperation(StencilOperation_t op)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilCompareFunction(StencilComparisonFunction_t fn)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilReferenceValue(int ref)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilTestMask(uint32 mask)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStencilWriteMask(uint32 mask)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ClearStencilBufferRectangle(int xmin, int ymin, int xmax, int ymax, int value)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::DisableAllLocalLights()
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::CompareSnapshots(StateSnapshot_t lhs, StateSnapshot_t rhs)
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

IMesh* ShaderAPI::GetFlexMesh()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool ShaderAPI::SupportsMSAAMode(int msaaMode)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderAPI::OwnGPUResources(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::BeginPIXEvent(unsigned long color, const char* szName)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EndPIXEvent()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetPIXMarker(unsigned long color, const char* szName)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EnableAlphaToCoverage()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::DisableAlphaToCoverage()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ComputeVertexDescription(unsigned char* buffer, VertexFormat_t fmt, MeshDesc_t& desc) const
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderAPI::SupportsShadowDepthTextures()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::SetDisallowAccess(bool disallowed)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EnableShaderShaderMutex(bool enabled)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ShaderLock()
{
	LOG_FUNC();
	m_ShaderLock.lock();
}

void ShaderAPI::ShaderUnlock()
{
	LOG_FUNC();
	m_ShaderLock.unlock();
}

ImageFormat ShaderAPI::GetShadowDepthTextureFormat()
{
	NOT_IMPLEMENTED_FUNC();
	return ImageFormat();
}

bool ShaderAPI::SupportsFetch4()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::SetShadowDepthBiasFactors(float slopeScaleDepthBias, float shadowDepthBias)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindVertexBuffer(int streamID, IVertexBuffer* vtxBuf, int byteOffset, int firstVertex, int vtxCount, VertexFormat_t fmt, int repetitions)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindIndexBuffer(IIndexBuffer* indexBuf, int offsetInBytes)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Draw(MaterialPrimitiveType_t type, int firstIndex, int indexCount)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PerformFullScreenStencilOperation()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetScissorRect(const int left, const int top, const int right, const int bottom, const bool enableScissor)
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderAPI::SupportsCSAAMode(int numSamples, int qualityLevel)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::InvalidateDelayedShaderConstants()
{
	NOT_IMPLEMENTED_FUNC();
}

float ShaderAPI::GammaToLinear_HardwareSpecific(float gamma) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

float ShaderAPI::LinearToGamma_HardwareSpecific(float linear) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

void ShaderAPI::SetLinearToGammaConversionTextures(ShaderAPITextureHandle_t srgbWriteEnabledTex, ShaderAPITextureHandle_t identityTex)
{
	NOT_IMPLEMENTED_FUNC();
}

ImageFormat ShaderAPI::GetNullTextureFormat()
{
	NOT_IMPLEMENTED_FUNC();
	return ImageFormat();
}

void ShaderAPI::BindVertexTexture(VertexTextureSampler_t sampler, ShaderAPITextureHandle_t tex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::EnableHWMorphing(bool enabled)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetFlexWeights(int firstWeight, int count, const MorphWeight_t* weights)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::AcquireThreadOwnership()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ReleaseThreadOwnership()
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderAPI::SupportsNormalMapCompression() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::EnableBuffer2FramesAhead(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetDepthFeatheringPixelShaderConstant(int constant, float depthBlendScale)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PrintfVA(char* fmt, va_list vargs)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::Printf(PRINTF_FORMAT_STRING const char* fmt, ...)
{
	NOT_IMPLEMENTED_FUNC();
}

float ShaderAPI::Knob(char* knobName, float* setValue)
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

void ShaderAPI::OverrideAlphaWriteEnable(bool enable, bool alphaWriteEnable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::OverrideColorWriteEnable(bool overrideEnable, bool colorWriteEnable)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::LockRect(void** outBits, int* outPitch, ShaderAPITextureHandle_t tex, int mipLevel, int x, int y, int w, int h, bool write, bool read)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::UnlockRect(ShaderAPITextureHandle_t tex, int mipLevel)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindStandardTexture(Sampler_t sampler, StandardTextureId_t id)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::BindStandardVertexTexture(VertexTextureSampler_t sampler, StandardTextureId_t id)
{
	NOT_IMPLEMENTED_FUNC();
}

ITexture* ShaderAPI::GetRenderTargetEx(int renderTargetID)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderAPI::SetToneMappingScaleLinear(const Vector& scale)
{
	NOT_IMPLEMENTED_FUNC();
}

const Vector& ShaderAPI::GetToneMappingScaleLinear() const
{
	NOT_IMPLEMENTED_FUNC();
	// TODO: insert return statement here
	return vec3_origin;
}

float ShaderAPI::GetLightMapScaleFactor() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

void ShaderAPI::LoadBoneMatrix(int boneIndex, const float* m)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetDXLevelDefaults(uint& dxLevelMax, uint& dxLevelRecommended)
{
	NOT_IMPLEMENTED_FUNC();
}

float ShaderAPI::GetAmbientLightCubeLuminance()
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

void ShaderAPI::GetDX9LightState(LightState_t* state) const
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetPixelFogCombo()
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool ShaderAPI::IsHWMorphingEnabled() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::GetStandardTextureDimensions(int* width, int* height, StandardTextureId_t id)
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderAPI::ShouldWriteDepthToDestAlpha() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderAPI::PushDeformation(const DeformationBase_t* deformation)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::PopDeformation()
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderAPI::GetNumActiveDeformations() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int ShaderAPI::GetPackedDeformationInformation(int maxOfUnderstoodDeformations, float* constantValuesOut, int bufSize, int maxDeformations, int* numDefsOut) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderAPI::MarkUnusedVertexFields(unsigned int flags, int texCoordCount, bool* unusedTexCoords)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::ExecuteCommandBuffer(uint8* cmdBuf)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetStandardTextureHandle(StandardTextureId_t id, ShaderAPITextureHandle_t tex)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::GetCurrentColorCorrection(ShaderColorCorrectionInfo_t* info)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::SetPSNearAndFarZ(int psRegister)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::TexLodClamp(int something)
{
	LOG_FUNC();

	if (something != 0)
		NOT_IMPLEMENTED_FUNC();
}

void ShaderAPI::TexLodBias(float bias)
{
	LOG_FUNC();

	if (bias != 0)
		NOT_IMPLEMENTED_FUNC();
}
