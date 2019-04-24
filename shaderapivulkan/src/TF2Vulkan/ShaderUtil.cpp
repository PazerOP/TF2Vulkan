#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/Placeholders.h>

#include <shaderapi/ishaderutil.h>

namespace
{
	class ShaderUtil final : public CBaseAppSystem<IShaderUtil>
	{
	public:
		void* QueryInterface(const char* interfaceName) override;

		MaterialSystem_Config_t& GetConfig() override;

		bool ConvertImageFormat(
			uint8* src, ImageFormat srcFormat,
			uint8* dst, ImageFormat dstFormat,
			int width, int height, int srcStride, int dstStride) override;

		int GetMemRequired(int width, int height, int depth, ImageFormat format, bool mipmap) override;

		const ImageFormatInfo_t& ImageFormatInfo(ImageFormat fmt) const override;

		void BindStandardTexture(Sampler_t sampler, StandardTextureId_t id) override;

		void GetLightmapDimensions(int* w, int* h) override;

		void ReleaseShaderObjects() override;
		void RestoreShaderObjects(CreateInterfaceFn shaderFactory, int changeFlags) override;

		bool IsInStubMode() override;
		bool InFlashlightMode() const override;
		bool InEditorMode() const override;

		void NoteAnisotropicLevel(int currentLevel) override;

		MorphFormat_t GetBoundMorphFormat() override;

		ITexture* GetRenderTargetEx(int rtID) override;

		void DrawClearBufferQuad(uint8 r, uint8 g, uint8 b, uint8 a,
			bool clearColor, bool clearAlpha, bool clearDepth) override;

		bool OnDrawMesh(IMesh* mesh, int firstIndex, int numIndices) override;
		bool OnDrawMesh(IMesh* mesh, CPrimList* lists, int listCount) override;
		bool OnSetFlexMesh(IMesh* staticMesh, IMesh* mesh, int vertexOffsetInBytes) override;
		bool OnSetColorMesh(IMesh* staticMesh, IMesh* mesh, int vertexOffsetInBytes) override;
		bool OnSetPrimitiveType(IMesh* mesh, MaterialPrimitiveType_t type) override;
		bool OnFlushBufferedPrimitives() override;

		void SyncMatrices() override;
		void SyncMatrix(MaterialMatrixMode_t mode) override;

		void BindStandardVertexTexture(VertexTextureSampler_t sampler, StandardTextureId_t id) override;
		void GetStandardTextureDimensions(int* width, int* height, StandardTextureId_t id) override;

		int MaxHWMorphBatchCount() const override;

		void GetCurrentColorCorrection(ShaderColorCorrectionInfo_t* info) override;

		void OnThreadEvent(uint32 threadEvent) override;

		MaterialThreadMode_t GetThreadMode() override;
		bool IsRenderThreadSafe() override;

		void UncacheUnusedMaterials(bool recomputeStateSnapshots) override;
	};
}

EXPOSE_SINGLE_INTERFACE(ShaderUtil, IShaderUtil, SHADER_UTIL_INTERFACE_VERSION);

void* ShaderUtil::QueryInterface(const char* interfaceName)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

MaterialSystem_Config_t& ShaderUtil::GetConfig()
{
	NOT_IMPLEMENTED_FUNC();
	// TODO: insert return statement here
	return *(MaterialSystem_Config_t*)nullptr;
}

bool ShaderUtil::ConvertImageFormat(uint8* src, ImageFormat srcFormat, uint8* dst, ImageFormat dstFormat, int width, int height, int srcStride, int dstStride)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int ShaderUtil::GetMemRequired(int width, int height, int depth, ImageFormat format, bool mipmap)
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

const ImageFormatInfo_t& ShaderUtil::ImageFormatInfo(ImageFormat fmt) const
{
	NOT_IMPLEMENTED_FUNC();
	// TODO: insert return statement here
	return *(const ImageFormatInfo_t*)nullptr;
}

void ShaderUtil::BindStandardTexture(Sampler_t sampler, StandardTextureId_t id)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::GetLightmapDimensions(int* w, int* h)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::ReleaseShaderObjects()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::RestoreShaderObjects(CreateInterfaceFn shaderFactory, int changeFlags)
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderUtil::IsInStubMode()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::InFlashlightMode() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::InEditorMode() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderUtil::NoteAnisotropicLevel(int currentLevel)
{
	NOT_IMPLEMENTED_FUNC();
}

MorphFormat_t ShaderUtil::GetBoundMorphFormat()
{
	NOT_IMPLEMENTED_FUNC();
	return MorphFormat_t();
}

ITexture* ShaderUtil::GetRenderTargetEx(int rtID)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderUtil::DrawClearBufferQuad(uint8 r, uint8 g, uint8 b, uint8 a, bool clearColor, bool clearAlpha, bool clearDepth)
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderUtil::OnDrawMesh(IMesh* mesh, int firstIndex, int numIndices)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::OnDrawMesh(IMesh* mesh, CPrimList* lists, int listCount)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::OnSetFlexMesh(IMesh* staticMesh, IMesh* mesh, int vertexOffsetInBytes)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::OnSetColorMesh(IMesh* staticMesh, IMesh* mesh, int vertexOffsetInBytes)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::OnSetPrimitiveType(IMesh* mesh, MaterialPrimitiveType_t type)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool ShaderUtil::OnFlushBufferedPrimitives()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderUtil::SyncMatrices()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::SyncMatrix(MaterialMatrixMode_t mode)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::BindStandardVertexTexture(VertexTextureSampler_t sampler, StandardTextureId_t id)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::GetStandardTextureDimensions(int* width, int* height, StandardTextureId_t id)
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderUtil::MaxHWMorphBatchCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderUtil::GetCurrentColorCorrection(ShaderColorCorrectionInfo_t* info)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderUtil::OnThreadEvent(uint32 threadEvent)
{
	NOT_IMPLEMENTED_FUNC();
}

MaterialThreadMode_t ShaderUtil::GetThreadMode()
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialThreadMode_t();
}

bool ShaderUtil::IsRenderThreadSafe()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderUtil::UncacheUnusedMaterials(bool recomputeStateSnapshots)
{
	NOT_IMPLEMENTED_FUNC();
}
