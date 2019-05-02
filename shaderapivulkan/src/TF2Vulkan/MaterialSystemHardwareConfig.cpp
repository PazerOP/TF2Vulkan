#include "ShaderDeviceMgr.h"
#include "MaterialSystemHardwareConfig.h"

#include <TF2Vulkan/Util/interface.h>

using namespace TF2Vulkan;

namespace
{
	class MaterialSystemHardwareConfig : public IMaterialSystemHardwareConfigInternal
	{
	public:
		bool HasDestAlphaBuffer() const override;
		bool HasStencilBuffer() const override;
		int GetFrameBufferColorDepth() const override;
		int GetSamplerCount() const override;
		bool HasSetDeviceGammaRamp() const override;
		bool SupportsCompressedTextures() const override;
		VertexCompressionType_t SupportsCompressedVertices() const override;
		bool SupportsNormalMapCompression() const override;
		bool SupportsVertexAndPixelShaders() const override;
		bool SupportsPixelShaders_1_4() const override;
		bool SupportsStaticControlFlow() const override;
		bool SupportsPixelShaders_2_0() const override;
		bool SupportsVertexShaders_2_0() const override;
		int MaximumAnisotropicLevel() const override;
		int MaxTextureWidth() const override;
		int MaxTextureHeight() const override;
		int TextureMemorySize() const override;
		bool SupportsOverbright() const override;
		bool SupportsCubeMaps() const override;
		bool SupportsMipmappedCubemaps() const override;
		bool SupportsNonPow2Textures() const override;

		int GetTextureStageCount() const override;
		int NumVertexShaderConstants() const override;
		int NumPixelShaderConstants() const override;
		int MaxNumLights() const override;
		bool SupportsHardwareLighting() const override;
		int MaxBlendMatrices() const override;
		int MaxBlendMatrixIndices() const override;
		int MaxTextureAspectRatio() const override;
		int MaxVertexShaderBlendMatrices() const override;
		int MaxUserClipPlanes() const override;
		bool UseFastClipping() const override;

		int GetDXSupportLevel() const override;
		const char* GetShaderDLLName() const override;

		bool ReadPixelsFromFrontBuffer() const override;

		bool PreferDynamicTextures() const override;

		bool SupportsHDR() const override;

		bool HasProjectedBumpEnv() const override;
		bool SupportsSpheremapping() const override;
		bool NeedsAAClamp() const override;
		bool NeedsATICentroidHack() const override;

		bool SupportsColorOnSecondStream() const override;
		bool SupportsStaticPlusDynamicLighting() const override;

		bool PreferReducedFillrate() const override;

		int GetMaxDXSupportLevel() const override;

		bool SpecifiesFogColorInLinearSpace() const override;

		bool SupportsSRGB() const override;
		bool FakeSRGBWrite() const override;
		bool CanDoSRGBReadFromRTs() const override;

		bool SupportsGLMixedSizeTargets() const override;

		bool IsAAEnabled() const override;

		int GetVertexTextureCount() const override;
		int GetMaxVertexTextureDimension() const override;

		int MaxTextureDepth() const override;

		HDRType_t GetHDRType() const override;
		HDRType_t GetHardwareHDRType() const override;

		bool SupportsPixelShaders_2_b() const override;
		bool SupportsStreamOffset() const override;

		int StencilBufferBits() const override;
		int MaxViewports() const override;

		void OverrideStreamOffsetSupport(bool bOverrideEnabled, bool bEnableSupport) override;

		int GetShadowFilterMode() const override;

		bool NeedsShaderSRGBConversionImpl() const override;

		bool UsesSRGBCorrectBlending() const override;

		bool SupportsShaderModel_3_0() const override;
		bool HasFastVertexTextures() const override;
		int MaxHWMorphBatchCount() const override;

		bool ActuallySupportsPixelShaders_2_b() const override;

		bool SupportsHDRMode(HDRType_t mode) const override;

		bool GetHDREnabled() const override;
		void SetHDREnabled(bool bEnable) override;

		bool SupportsBorderColor() const override;
		bool SupportsFetch4() const override;

		bool CanStretchRectFromTextures() const override;

		const char* GetHWSpecificShaderDLLName() const override;

		int NumBooleanVertexShaderConstants() const override;
		int NumIntegerVertexShaderConstants() const override;
		int NumBooleanPixelShaderConstants() const override;
		int NumIntegerPixelShaderConstants() const override;

		uint32_t MaxVertexAttributes() const override;

		void Init() override;

	private:
		HDRType_t m_HDRType = HDRType_t::HDR_TYPE_FLOAT;

		bool m_Init = false;
		vk::PhysicalDeviceLimits m_Limits;
		vk::PhysicalDeviceFeatures m_Features;

		const vk::PhysicalDeviceLimits& GetLimits() const;
		const vk::PhysicalDeviceFeatures& GetFeatures() const;
	};
}

static MaterialSystemHardwareConfig s_HardwareConfig;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(MaterialSystemHardwareConfig, IMaterialSystemHardwareConfig,
	MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, s_HardwareConfig);

IMaterialSystemHardwareConfigInternal& TF2Vulkan::g_MatSysConfig = s_HardwareConfig;

bool MaterialSystemHardwareConfig::HasDestAlphaBuffer() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::HasStencilBuffer() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int MaterialSystemHardwareConfig::GetFrameBufferColorDepth() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::GetSamplerCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool MaterialSystemHardwareConfig::HasSetDeviceGammaRamp() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsCompressedTextures() const
{
	LOG_FUNC();
	return GetFeatures().textureCompressionBC;
}

VertexCompressionType_t MaterialSystemHardwareConfig::SupportsCompressedVertices() const
{
	NOT_IMPLEMENTED_FUNC();
	return VertexCompressionType_t::VERTEX_COMPRESSION_NONE;
}

bool MaterialSystemHardwareConfig::SupportsNormalMapCompression() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsVertexAndPixelShaders() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsPixelShaders_1_4() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsStaticControlFlow() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsPixelShaders_2_0() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsVertexShaders_2_0() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

int MaterialSystemHardwareConfig::MaximumAnisotropicLevel() const
{
	LOG_FUNC();
	return GetLimits().maxSamplerAnisotropy;
}

int MaterialSystemHardwareConfig::MaxTextureWidth() const
{
	LOG_FUNC();
	return GetLimits().maxImageDimension2D;
}

int MaterialSystemHardwareConfig::MaxTextureHeight() const
{
	LOG_FUNC();
	return GetLimits().maxImageDimension2D;
}

int MaterialSystemHardwareConfig::TextureMemorySize() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool MaterialSystemHardwareConfig::SupportsOverbright() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsCubeMaps() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsMipmappedCubemaps() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsNonPow2Textures() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

int MaterialSystemHardwareConfig::GetTextureStageCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::NumVertexShaderConstants() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::NumPixelShaderConstants() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::MaxNumLights() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool MaterialSystemHardwareConfig::SupportsHardwareLighting() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int MaterialSystemHardwareConfig::MaxBlendMatrices() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::MaxBlendMatrixIndices() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::MaxTextureAspectRatio() const
{
	LOG_FUNC();
	return MaxTextureWidth();
}

int MaterialSystemHardwareConfig::MaxVertexShaderBlendMatrices() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::MaxUserClipPlanes() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool MaterialSystemHardwareConfig::UseFastClipping() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int MaterialSystemHardwareConfig::GetDXSupportLevel() const
{
	LOG_FUNC();
	return GetMaxDXSupportLevel();
}

const char* MaterialSystemHardwareConfig::GetShaderDLLName() const
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool MaterialSystemHardwareConfig::ReadPixelsFromFrontBuffer() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::PreferDynamicTextures() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsHDR() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::HasProjectedBumpEnv() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsSpheremapping() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::NeedsAAClamp() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::NeedsATICentroidHack() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsColorOnSecondStream() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsStaticPlusDynamicLighting() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::PreferReducedFillrate() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int MaterialSystemHardwareConfig::GetMaxDXSupportLevel() const
{
	LOG_FUNC();
	// TODO: What directx version is comparable with vulkan minimum requirements?

	// NOTE: Don't say anything over 99 otherwise the material system will try
	// to load stdshader_dx10.dll
	return 99;
}

bool MaterialSystemHardwareConfig::SpecifiesFogColorInLinearSpace() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsSRGB() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::FakeSRGBWrite() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::CanDoSRGBReadFromRTs() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::SupportsGLMixedSizeTargets() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::IsAAEnabled() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int MaterialSystemHardwareConfig::GetVertexTextureCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::GetMaxVertexTextureDimension() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::MaxTextureDepth() const
{
	LOG_FUNC();
	return GetLimits().maxImageDimension3D;
}

HDRType_t MaterialSystemHardwareConfig::GetHDRType() const
{
	LOG_FUNC();
	return m_HDRType;
}

HDRType_t MaterialSystemHardwareConfig::GetHardwareHDRType() const
{
	LOG_FUNC();
	return HDRType_t::HDR_TYPE_FLOAT;
}

bool MaterialSystemHardwareConfig::SupportsPixelShaders_2_b() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsStreamOffset() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int MaterialSystemHardwareConfig::StencilBufferBits() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int MaterialSystemHardwareConfig::MaxViewports() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void MaterialSystemHardwareConfig::OverrideStreamOffsetSupport(bool overrideEnabled, bool enableSupport)
{
	NOT_IMPLEMENTED_FUNC();
}

int MaterialSystemHardwareConfig::GetShadowFilterMode() const
{
	NOT_IMPLEMENTED_FUNC();
	return -1;
}

bool MaterialSystemHardwareConfig::NeedsShaderSRGBConversionImpl() const
{
	LOG_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::UsesSRGBCorrectBlending() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsShaderModel_3_0() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::HasFastVertexTextures() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

int MaterialSystemHardwareConfig::MaxHWMorphBatchCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool MaterialSystemHardwareConfig::ActuallySupportsPixelShaders_2_b() const
{
	LOG_FUNC();
	return true;  // All vulkan hardware supports this
}

bool MaterialSystemHardwareConfig::SupportsHDRMode(HDRType_t mode) const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::GetHDREnabled() const
{
	LOG_FUNC();
	return GetHDRType() != HDRType_t::HDR_TYPE_NONE;
}

void MaterialSystemHardwareConfig::SetHDREnabled(bool enabled)
{
	NOT_IMPLEMENTED_FUNC();
}

bool MaterialSystemHardwareConfig::SupportsBorderColor() const
{
	LOG_FUNC();
	return false; // Not unless you happen to want one of the few predefined ones
}

bool MaterialSystemHardwareConfig::SupportsFetch4() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool MaterialSystemHardwareConfig::CanStretchRectFromTextures() const
{
	LOG_FUNC();
	return true; // I mean, probably
}

const char* MaterialSystemHardwareConfig::GetHWSpecificShaderDLLName() const
{
	LOG_FUNC();
	// TODO: Do we want/need to take advantage of this?
	//NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

const vk::PhysicalDeviceLimits& MaterialSystemHardwareConfig::GetLimits() const
{
	assert(m_Init);
	return m_Limits;
}

const vk::PhysicalDeviceFeatures& MaterialSystemHardwareConfig::GetFeatures() const
{
	assert(m_Init);
	return m_Features;
}

int MaterialSystemHardwareConfig::NumBooleanVertexShaderConstants() const
{
	LOG_FUNC();
	// Since we're really using constant buffers... "max constants" doesn't
	// really mean anything anymore
	return INT_MAX;
}

int MaterialSystemHardwareConfig::NumIntegerVertexShaderConstants() const
{
	LOG_FUNC();
	// Since we're really using constant buffers... "max constants" doesn't
	// really mean anything anymore
	return INT_MAX;
}

int MaterialSystemHardwareConfig::NumBooleanPixelShaderConstants() const
{
	LOG_FUNC();
	// Since we're really using constant buffers... "max constants" doesn't
	// really mean anything anymore
	return INT_MAX;
}

int MaterialSystemHardwareConfig::NumIntegerPixelShaderConstants() const
{
	LOG_FUNC();
	// Since we're really using constant buffers... "max constants" doesn't
	// really mean anything anymore
	return INT_MAX;
}

uint32_t MaterialSystemHardwareConfig::MaxVertexAttributes() const
{
	return GetLimits().maxVertexInputAttributes;
}

void MaterialSystemHardwareConfig::Init()
{
	assert(!m_Init);

	auto adapter = g_ShaderDeviceMgr.GetAdapter();
	m_Limits = adapter.getProperties().limits;
	m_Features = adapter.getFeatures();

	m_Init = true;
}
