#include "TF2Vulkan/DebugTextureInfo.h"
#include "TF2Vulkan/Material.h"
#include "TF2Vulkan/MaterialSystem.h"
#include "TF2Vulkan/MaterialSystemHardwareConfig.h"
#include "Util/KeyValues.h"
#include "Util/Placeholders.h"
#include "Util/std_algorithm.h"

#include <filesystem.h>
#include <materialsystem/idebugtextureinfo.h>
#include <materialsystem/imaterialsystem.h>
#include <tier0/icommandline.h>
#include <tier1/KeyValues.h>
#include <tier1/tier1.h>
#include <tier1/utlbuffer.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>

#include <vulkan/vulkan.hpp>

#include <string>
#include <unordered_map>
#include <vector>

using namespace TF2Vulkan;

namespace
{
	class VulkanMaterialSystem final : public IMaterialSystem
	{
	public:
		///////////////////////////////
		// IAppSystem Implementation //
		///////////////////////////////
		bool Connect(CreateInterfaceFn factory) override;
		void Disconnect() override;

		void* QueryInterface(const char* interfaceName) override;

		InitReturnVal_t Init() override;
		void Shutdown() override;

		////////////////////////////////////
		// IMaterialSystem Implementation //
		////////////////////////////////////

		CreateInterfaceFn Init(const char* shaderAPIDLL,
			IMaterialProxyFactory* materialProxyFactory,
			CreateInterfaceFn fileSystemFactory,
			CreateInterfaceFn cvarFactory) override;

		void SetShaderAPI(const char* shaderAPIDLL) override;

		void SetAdapter(int adapter, int flags) override;
		void SetAdapter(int adapter, MaterialInitFlags_t flags);

		void ModInit() override;
		void ModShutdown() override;

		void SetThreadMode(MaterialThreadMode_t mode, int serviceThread) override;
		MaterialThreadMode_t GetThreadMode() override;
		bool IsRenderThreadSafe() override;
		void ExecuteQueued() override;

		IMaterialSystemHardwareConfig* GetHardwareConfig(const char* version, int* returnCode) override;

		bool UpdateConfig(bool forceUpdate) override;

		bool OverrideConfig(const MaterialSystem_Config_t& config, bool forceUpdate) override;

		const MaterialSystem_Config_t& GetCurrentConfigForVideoCard() const override;

		bool GetRecommendedConfigurationInfo(int dxLevel, KeyValues* keyValues) override;

		int GetDisplayAdapterCount() const override;
		int GetCurrentAdapter() const override;
		void GetDisplayAdapterInfo(int adapter, MaterialAdapterInfo_t& info) const override;
		int GetModeCount(int adapter) const override;
		void GetModeInfo(int adapter, int mode, MaterialVideoMode_t& info) const override;
		void AddModeChangeCallBack(ModeChangeCallbackFunc_t func) override;
		void RemoveModeChangeCallBack(ModeChangeCallbackFunc_t func) override;
		void GetDisplayMode(MaterialVideoMode_t& mode) const override;
		bool SetMode(void* hwnd, const MaterialSystem_Config_t& config) override;
		bool SupportsMSAAMode(int msaaMode) override;
		const MaterialSystemHardwareIdentifier_t& GetVideoCardIdentifier() const override;
		void SpewDriverInfo() const override;
		void GetDXLevelDefaults(uint& dxLevelMax, uint& recommendedDXLevel) override;
		void GetBackBufferDimensions(int& width, int& height) const override;
		ImageFormat GetBackBufferFormat() const override;
		bool SupportsHDRMode(HDRType_t hdrMode) override;

		bool AddView(void* hwnd) override;
		void RemoveView(void* hwnd) override;
		void SetView(void* hwnd) override;

		void BeginFrame(float frametime) override;
		void EndFrame() override;
		void Flush(bool flushHardware) override;

		void SwapBuffers() override;

		void EvictManagedResources() override;

		void ReleaseResources() override;
		void ReacquireResources() override;

		void AddReleaseFunc(MaterialBufferReleaseFunc_t func) override;
		void RemoveReleaseFunc(MaterialBufferReleaseFunc_t func) override;

		void AddRestoreFunc(MaterialBufferRestoreFunc_t func) override;
		void RemoveRestoreFunc(MaterialBufferRestoreFunc_t func) override;

		void ResetTempHWMemory(bool exitingLevel) override;

		void HandleDeviceLost() override;

		int ShaderCount() const override;
		int GetShaders(int firstShader, int maxCount, IShader** shaderList) const override;

		int ShaderFlagCount() const override;
		const char* ShaderFlagName(int index) const override;

		void GetShaderFallback(const char* shaderName, char* fallbackShader, int fallbackShaderLength) override;

		IMaterialProxyFactory* GetMaterialProxyFactory() override;
		void SetMaterialProxyFactory(IMaterialProxyFactory* factory) override;

		void EnableEditorMaterials() override;

		void SetInStubMode(bool inStubMode) override;

		void DebugPrintUsedMaterials(const char* searchSubString, bool verbose) override;
		void DebugPrintUsedTextures() override;

		void ToggleSuppressMaterial(const char* materialName) override;
		void ToggleDebugMaterial(const char* materialName) override;

		bool UsingFastClipping() override;
		int StencilBufferBits() override;

		void SuspendTextureStreaming() override;
		void ResumeTextureStreaming() override;

		void UncacheAllMaterials() override;
		void UncacheUnusedMaterials(bool recomputeStateSnapshots) override;
		void CacheUsedMaterials() override;
		void ReloadTextures() override;
		void ReloadMaterials(const char* subString) override;
		IMaterial* CreateMaterial(const char* materialName, KeyValues* vmtKeyValues) override;
		IMaterial* FindMaterial(const char* materialName, const char* textureGroupName,
			bool complain, const char* complainPrefix) override;
		IMaterial* FindMaterialEx(const char* materialName, const char* textureGroupName,
			int context, bool complain, const char* complainPrefix) override;
		bool IsMaterialLoaded(const char* materialName) override;
		MaterialHandle_t FirstMaterial() const override;
		MaterialHandle_t NextMaterial(MaterialHandle_t h) const override;
		MaterialHandle_t InvalidMaterial() const override;
		IMaterial* GetMaterial(MaterialHandle_t h) const override;
		int GetNumMaterials() const override;
		IMaterial* FindProceduralMaterial(const char* materialName, const char* textureGroupName,
			KeyValues* vmtKeyValues) override;

		void SetAsyncTextureLoadCache(void* fileCache) override;

		ITexture* FindTexture(const char* textureName, const char* textureGroupName,
			bool complain, int additionalCreationFlags) override;
		void AsyncFindTexture(const char* fileName, const char* textureGroupName,
			IAsyncTextureOperationReceiver* recipient, void* extraArgs, bool complain,
			int additionalCreationFlags) override;
		ITexture* CreateTextureFromBits(int w, int h, int mips, ImageFormat fmt,
			int srcBufferSize, byte* srcBits) override;
		ITexture* CreateNamedTextureFromBitsEx(const char* name, const char* textureGroupName,
			int w, int h, int mips, ImageFormat fmt, int srcBufferSize, byte* srcBits, int flags) override;
		bool IsTextureLoaded(const char* textureName) const override;
		ITexture* CreateProceduralTexture(const char* textureName, const char* textureGroupName,
			int w, int h, ImageFormat fmt, int flags) override;

		void BeginRenderTargetAllocation() override;
		void EndRenderTargetAllocation() override;
		void OverrideRenderTargetAllocation(bool rtAlloc) override;
		ITexture* CreateRenderTargetTexture(int w, int h, RenderTargetSizeMode_t sizeMode,
			ImageFormat format, MaterialRenderTargetDepth_t depth) override;
		ITexture* CreateNamedRenderTargetTexture(const char* rtName, int w, int h,
			RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth,
			bool clampTexCoords = true, bool autoMipMap = false) override;
		ITexture* CreateNamedRenderTargetTextureEx(const char* rtName, int w, int h,
			RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth,
			unsigned int textureFlags, unsigned int renderTargetFlags) override;
		ITexture* CreateNamedRenderTargetTextureEx2(const char* rtName, int w, int h,
			RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth,
			unsigned int textureFlags, unsigned int renderTargetFlags) override;

		void BeginLightmapAllocation() override;
		void EndLightmapAllocation() override;
		void BeginUpdateLightmaps() override;
		void EndUpdateLightmaps() override;
		int AllocateLightmap(int width, int height, int offsetIntoLightmapPage[2],
			IMaterial* material) override;
		int AllocateWhiteLightmap(IMaterial* material) override;
		void UpdateLightmap(int lightmapPageID, int lightmapSize[2],
			int offsetIntoLightmapPage[2], float* floatImage, float* floatImagebump1,
			float* floatImageBump2, float* floatImageBump3) override;
		void GetLightmapPageSize(int lightmap, int* width, int* height) const override;
		void ResetMaterialLightmapPageInfo() override;
		int AllocateDynamicLightmap(int lightmapSize[2], int* outOffsetIntoPage,
			int frameID) override;

		int GetNumSortIDs() override;
		void GetSortInfo(MaterialSystem_SortInfo_t* sortInfoArray) override;

		void ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil) override;

		bool SupportsShadowDepthTextures() override;
		ImageFormat GetShadowDepthTextureFormat() override;

		MaterialLock_t Lock() override;
		void Unlock(MaterialLock_t lock) override;

		bool SupportsFetch4() override;

		IMatRenderContext* GetRenderContext() override;
		IMatRenderContext* CreateRenderContext(MaterialContextType_t type) override;
		IMatRenderContext* SetRenderContext(IMatRenderContext* newCtx) override;

		bool SupportsCSAAMode(int numSamples, int qualityLevel) override;

		ImageFormat GetNullTextureFormat() override;

		void AddTextureAlias(const char* alias, const char* realName) override;
		void RemoveTextureAlias(const char* alias) override;

		void SetExcludedTextures(const char* scriptName) override;
		void UpdateExcludedTextures() override;

		bool IsInFrame() const override;

		void CompactMemory() override;

		void ReloadFilesInList(IFileList* filesToReload) override;
		bool AllowThreading(bool allow, int serviceThread) override;

		void SetRenderTargetFrameBufferSizeOverrides(int width, int height) override;
		void GetRenderTargetFrameBufferDimensions(int& width, int& height) override;

		char* GetDisplayDeviceName() const override;

		ITextureCompositor* NewTextureCompositor(int w, int h, const char* compositeName,
			int teamNum, uint64 randomSeed, KeyValues* stageDesc, uint32 texCompositeCreateFlags) override;

	private:
		IMaterial* CreateMaterial(const CUtlSymbolDbg& materialName, KeyValues* vmtKeyValues, const CUtlSymbolDbg& textureGroupName,
			bool complain = false, const char* complainPrefix = nullptr);

		// This is what was set, but probably not what we're actually using
		std::string m_ShaderAPIDLL;

		std::vector<ModeChangeCallbackFunc_t> m_ModeChangeCallbacks;
		std::vector<MaterialBufferReleaseFunc_t> m_ReleaseFuncs;
		std::vector<MaterialBufferRestoreFunc_t> m_RestoreFuncs;

		bool m_HasInit = false;
		int m_Adapter = 0;
		MaterialInitFlags_t m_InitFlags{};
		IMaterialProxyFactory* m_ProxyFactory = nullptr;

		std::unordered_map<CUtlSymbolDbg, std::unique_ptr<Material>> m_Materials;
	};
}

static VulkanMaterialSystem s_MaterialSystem;
IMaterialSystem* TF2Vulkan::GetMaterialSystem() { return &s_MaterialSystem; }

bool VulkanMaterialSystem::Connect(CreateInterfaceFn factory)
{
	ConnectTier1Libraries(&factory, 1);
	ConnectTier2Libraries(&factory, 1);
	ConnectTier3Libraries(&factory, 1);

	if (!g_pFullFileSystem)
	{
		Warning("[TF2Vulkan] Failed to hook up g_pFullFileSystem\n");
		assert(false);
		return false;
	}

	//NOT_IMPLEMENTED_FUNC();
	return true;
}

void VulkanMaterialSystem::Disconnect()
{
	NOT_IMPLEMENTED_FUNC();
}

void* VulkanMaterialSystem::QueryInterface(const char* interfaceName)
{
	if (!strcmp(interfaceName, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION))
		return TF2Vulkan::GetMaterialSystemHardwareConfig();
	else if (!strcmp(interfaceName, DEBUG_TEXTURE_INFO_VERSION))
		return TF2Vulkan::GetDebugTextureInfo();

	Msg("[TF2Vulkan] " __FUNCTION__ "(): Reporting interface %s as unavailable\n", interfaceName);
	return nullptr;
}

InitReturnVal_t VulkanMaterialSystem::Init()
{
	if (!CommandLine()->CheckParm("-insecure"))
		Error("The vulkan rendering backend is not officially supported by Valve. To avoid getting VAC banned, you must run with -insecure.");

	return INIT_OK;
}

void VulkanMaterialSystem::Shutdown()
{
	NOT_IMPLEMENTED_FUNC();
}

CreateInterfaceFn VulkanMaterialSystem::Init(const char* shaderAPIDLL, IMaterialProxyFactory* materialProxyFactory,
	CreateInterfaceFn fileSystemFactory, CreateInterfaceFn cvarFactory)
{
	NOT_IMPLEMENTED_FUNC();
	return CreateInterfaceFn();
}

void VulkanMaterialSystem::SetShaderAPI(const char* shaderAPIDLL)
{
	m_ShaderAPIDLL = shaderAPIDLL;
}

void VulkanMaterialSystem::SetAdapter(int adapter, int flags)
{
	return SetAdapter(adapter, MaterialInitFlags_t(flags));
}

void VulkanMaterialSystem::SetAdapter(int adapter, MaterialInitFlags_t flags)
{
	assert(!m_HasInit);
	m_Adapter = adapter;
	m_InitFlags = flags;
}

void VulkanMaterialSystem::ModInit()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ModShutdown()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::SetThreadMode(MaterialThreadMode_t mode, int serviceThread)
{
	NOT_IMPLEMENTED_FUNC();
}

MaterialThreadMode_t VulkanMaterialSystem::GetThreadMode()
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialThreadMode_t::MATERIAL_QUEUED_THREADED;
}

bool VulkanMaterialSystem::IsRenderThreadSafe()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void VulkanMaterialSystem::ExecuteQueued()
{
	NOT_IMPLEMENTED_FUNC();
}

IMaterialSystemHardwareConfig* VulkanMaterialSystem::GetHardwareConfig(const char* version, int* returnCode)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool VulkanMaterialSystem::UpdateConfig(bool forceUpdate)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool VulkanMaterialSystem::OverrideConfig(const MaterialSystem_Config_t& config, bool forceUpdate)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

const MaterialSystem_Config_t& VulkanMaterialSystem::GetCurrentConfigForVideoCard() const
{
	NOT_IMPLEMENTED_FUNC();
	return *(const MaterialSystem_Config_t*)nullptr;
}

bool VulkanMaterialSystem::GetRecommendedConfigurationInfo(int dxLevel, KeyValues* keyValues)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int VulkanMaterialSystem::GetDisplayAdapterCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int VulkanMaterialSystem::GetCurrentAdapter() const
{
	return m_Adapter;
}

void VulkanMaterialSystem::GetDisplayAdapterInfo(int adapter, MaterialAdapterInfo_t& info) const
{
	NOT_IMPLEMENTED_FUNC();
}

int VulkanMaterialSystem::GetModeCount(int adapter) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void VulkanMaterialSystem::GetModeInfo(int adapter, int mode, MaterialVideoMode_t& info) const
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::AddModeChangeCallBack(ModeChangeCallbackFunc_t func)
{
	assert(std::find(m_ModeChangeCallbacks.begin(), m_ModeChangeCallbacks.end(), func) == m_ModeChangeCallbacks.end());
	m_ModeChangeCallbacks.push_back(func);
}

void VulkanMaterialSystem::RemoveModeChangeCallBack(ModeChangeCallbackFunc_t func)
{
	Util::algorithm::try_erase(m_ModeChangeCallbacks, func);
}

void VulkanMaterialSystem::GetDisplayMode(MaterialVideoMode_t& mode) const
{
	NOT_IMPLEMENTED_FUNC();
}

bool VulkanMaterialSystem::SetMode(void* hwnd, const MaterialSystem_Config_t& config)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool VulkanMaterialSystem::SupportsMSAAMode(int msaaMode)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

const MaterialSystemHardwareIdentifier_t& VulkanMaterialSystem::GetVideoCardIdentifier() const
{
	NOT_IMPLEMENTED_FUNC();
	return *(const MaterialSystemHardwareIdentifier_t*)nullptr;
}

void VulkanMaterialSystem::SpewDriverInfo() const
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::GetDXLevelDefaults(uint& dxLevelMax, uint& recommendedDXLevel)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::GetBackBufferDimensions(int& width, int& height) const
{
	NOT_IMPLEMENTED_FUNC();
}

ImageFormat VulkanMaterialSystem::GetBackBufferFormat() const
{
	NOT_IMPLEMENTED_FUNC();
	return ImageFormat::IMAGE_FORMAT_UNKNOWN;
}

bool VulkanMaterialSystem::SupportsHDRMode(HDRType_t hdrMode)
{
	NOT_IMPLEMENTED_FUNC();
	return true;
}

bool VulkanMaterialSystem::AddView(void* hwnd)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void VulkanMaterialSystem::RemoveView(void* hwnd)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::SetView(void* hwnd)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::BeginFrame(float frametime)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::EndFrame()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::Flush(bool flushHardware)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::SwapBuffers()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::EvictManagedResources()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ReleaseResources()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ReacquireResources()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::AddReleaseFunc(MaterialBufferReleaseFunc_t func)
{
	assert(!Util::algorithm::contains(m_ReleaseFuncs, func));
	m_ReleaseFuncs.push_back(func);
}

void VulkanMaterialSystem::RemoveReleaseFunc(MaterialBufferReleaseFunc_t func)
{
	Util::algorithm::try_erase(m_ReleaseFuncs, func);
}

void VulkanMaterialSystem::AddRestoreFunc(MaterialBufferRestoreFunc_t func)
{
	assert(!Util::algorithm::contains(m_RestoreFuncs, func));
	m_RestoreFuncs.push_back(func);
}

void VulkanMaterialSystem::RemoveRestoreFunc(MaterialBufferRestoreFunc_t func)
{
	Util::algorithm::try_erase(m_RestoreFuncs, func);
}

void VulkanMaterialSystem::ResetTempHWMemory(bool exitingLevel)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::HandleDeviceLost()
{
	NOT_IMPLEMENTED_FUNC();
}

int VulkanMaterialSystem::ShaderCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int VulkanMaterialSystem::GetShaders(int firstShader, int amxCount, IShader** shaderList) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int VulkanMaterialSystem::ShaderFlagCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

const char* VulkanMaterialSystem::ShaderFlagName(int index) const
{
	NOT_IMPLEMENTED_FUNC();
	return __FUNCSIG__ " -> INVALID FLAG INDEX";
}

void VulkanMaterialSystem::GetShaderFallback(const char* shaderName, char* fallbackShader, int fallbackShaderLength)
{
	NOT_IMPLEMENTED_FUNC();

	if (fallbackShader && fallbackShaderLength > 0)
		fallbackShader[0] = '\0';
}

IMaterialProxyFactory* VulkanMaterialSystem::GetMaterialProxyFactory()
{
	return m_ProxyFactory;
}

void VulkanMaterialSystem::SetMaterialProxyFactory(IMaterialProxyFactory* factory)
{
	m_ProxyFactory = factory;
}

void VulkanMaterialSystem::EnableEditorMaterials()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::SetInStubMode(bool inStubMode)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::DebugPrintUsedMaterials(const char* searchSubString, bool verbose)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::DebugPrintUsedTextures()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ToggleSuppressMaterial(const char* materialName)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ToggleDebugMaterial(const char* materialName)
{
	NOT_IMPLEMENTED_FUNC();
}

bool VulkanMaterialSystem::UsingFastClipping()
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int VulkanMaterialSystem::StencilBufferBits()
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void VulkanMaterialSystem::SuspendTextureStreaming()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ResumeTextureStreaming()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::UncacheAllMaterials()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::UncacheUnusedMaterials(bool recomputeStateSnapshots)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::CacheUsedMaterials()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ReloadTextures()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ReloadMaterials(const char* subString)
{
	NOT_IMPLEMENTED_FUNC();
}

IMaterial* VulkanMaterialSystem::CreateMaterial(const char* materialName, KeyValues* vmtKeyValues)
{
	return CreateMaterial(CUtlSymbol(materialName), vmtKeyValues, TEXTURE_GROUP_OTHER, true);
}

IMaterial* VulkanMaterialSystem::CreateMaterial(const CUtlSymbolDbg& materialName, KeyValues* vmtKeyValues,
	const CUtlSymbolDbg& textureGroupName, bool complain, const char* complainPrefix)
{
	const CUtlSymbolDbg nameSymbol(materialName);
	auto newMaterial = std::make_unique<Material>(*vmtKeyValues, nameSymbol, textureGroupName);
	IMaterial* newMaterialPtr = newMaterial.get();
	m_Materials[nameSymbol] = std::move(newMaterial);

	return newMaterialPtr;
}

IMaterial* VulkanMaterialSystem::FindMaterial(const char* materialName, const char* textureGroupName,
	bool complain, const char* complainPrefix)
{
	const CUtlSymbolDbg nameSymbol(materialName);
	if (auto found = m_Materials.find(nameSymbol); found != m_Materials.end())
		return found->second.get();

	char materialFileNameBuf[MAX_PATH];
	sprintf_s(materialFileNameBuf, "%s.vmt", materialName);

	KeyValues::AutoDelete kv(materialFileNameBuf);
	if (!kv->LoadFromFile(g_pFullFileSystem, materialFileNameBuf))
	{
		Warning("Unable to read material from file \"%s\"\n", materialFileNameBuf);
		assert(false);
		return nullptr;
	}

	return CreateMaterial(nameSymbol, kv, CUtlSymbol(textureGroupName), complain, complainPrefix);
}

IMaterial* VulkanMaterialSystem::FindMaterialEx(const char* materialName, const char* textureGroupName,
	int context, bool complain, const char* complainPrefix)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool VulkanMaterialSystem::IsMaterialLoaded(const char* materialName)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

MaterialHandle_t VulkanMaterialSystem::FirstMaterial() const
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialHandle_t{};
}

MaterialHandle_t VulkanMaterialSystem::NextMaterial(MaterialHandle_t h) const
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialHandle_t{};
}

MaterialHandle_t VulkanMaterialSystem::InvalidMaterial() const
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialHandle_t{};
}

IMaterial* VulkanMaterialSystem::GetMaterial(MaterialHandle_t h) const
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

int VulkanMaterialSystem::GetNumMaterials() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

IMaterial* VulkanMaterialSystem::FindProceduralMaterial(const char* materialName, const char* textureGroupName,
	KeyValues* vmtKeyValues)
{
	const CUtlSymbol pooledName(materialName);
	if (auto found = m_Materials.find(pooledName); found != m_Materials.end())
		return found->second.get();

	auto newMaterial = std::make_unique<Material>(*vmtKeyValues, pooledName, CUtlSymbol(textureGroupName));
	IMaterial* newMaterialPtr = newMaterial.get();
	m_Materials[pooledName] = std::move(newMaterial);

	return newMaterialPtr;
}

void VulkanMaterialSystem::SetAsyncTextureLoadCache(void* fileCache)
{
	NOT_IMPLEMENTED_FUNC();
}

ITexture* VulkanMaterialSystem::FindTexture(const char* textureName, const char* textureGroupName,
	bool complain, int additionalCreationFlags)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void VulkanMaterialSystem::AsyncFindTexture(const char* fileName, const char* textureGroupName,
	IAsyncTextureOperationReceiver* recipient, void* extraArgs, bool complain, int additionaCreationFlags)
{
	NOT_IMPLEMENTED_FUNC();
}

ITexture* VulkanMaterialSystem::CreateTextureFromBits(int w, int h, int mips, ImageFormat fmt,
	int srcBufferSize, byte* srcBits)
{
	return CreateNamedTextureFromBitsEx(nullptr, nullptr, w, h, mips, fmt, srcBufferSize, srcBits, 0);
}

ITexture* VulkanMaterialSystem::CreateNamedTextureFromBitsEx(const char* name, const char* textureGroupName,
	int w, int h, int mips, ImageFormat fmt, int srcBufferSize, byte* srcBits, int flags)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool VulkanMaterialSystem::IsTextureLoaded(const char* textureName) const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

ITexture* VulkanMaterialSystem::CreateProceduralTexture(const char* textureName, const char* textureGroupName,
	int w, int h, ImageFormat fmt, int flags)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void VulkanMaterialSystem::BeginRenderTargetAllocation()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::EndRenderTargetAllocation()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::OverrideRenderTargetAllocation(bool rtAlloc)
{
	NOT_IMPLEMENTED_FUNC();
}

ITexture* VulkanMaterialSystem::CreateRenderTargetTexture(int w, int h, RenderTargetSizeMode_t sizeMode,
	ImageFormat format, MaterialRenderTargetDepth_t depth)
{
	return CreateNamedRenderTargetTexture(nullptr, w, h, sizeMode, format, depth);
}

ITexture* VulkanMaterialSystem::CreateNamedRenderTargetTexture(const char* rtName, int w, int h,
	RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth,
	bool clampTexCoords, bool autoMipMap)
{
	return CreateNamedRenderTargetTextureEx(rtName, w, h, sizeMode, format, depth,
		clampTexCoords ? TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_CLAMPU : 0,
		autoMipMap ? CREATERENDERTARGETFLAGS_AUTOMIPMAP : 0);
}

ITexture* VulkanMaterialSystem::CreateNamedRenderTargetTextureEx(const char* rtName, int w, int h,
	RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth,
	unsigned int textureFlags, unsigned int renderTargetFlags)
{
	return CreateNamedRenderTargetTextureEx2(rtName, w, h, sizeMode, format, depth,
		textureFlags, renderTargetFlags);
}


ITexture* VulkanMaterialSystem::CreateNamedRenderTargetTextureEx2(const char* rtName, int w, int h,
	RenderTargetSizeMode_t sizeMode, ImageFormat format, MaterialRenderTargetDepth_t depth,
	unsigned int textureFlags, unsigned int renderTargetFlags)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void VulkanMaterialSystem::BeginLightmapAllocation()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::EndLightmapAllocation()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::BeginUpdateLightmaps()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::EndUpdateLightmaps()
{
	NOT_IMPLEMENTED_FUNC();
}

int VulkanMaterialSystem::AllocateLightmap(int width, int height,
	int offsetIntoLightmapPage[2], IMaterial* material)
{
	NOT_IMPLEMENTED_FUNC();
	return -1;
}

int VulkanMaterialSystem::AllocateWhiteLightmap(IMaterial* material)
{
	NOT_IMPLEMENTED_FUNC();
	return -1;
}

void VulkanMaterialSystem::UpdateLightmap(int lightmapPageID, int lightmapSize[2],
	int offsetIntoLightmapPage[2], float* floatImage, float* floatImageBump1,
	float* floatImageBump2, float* floatImageBump3)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::GetLightmapPageSize(int lightmap, int* width, int* height) const
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ResetMaterialLightmapPageInfo()
{
	NOT_IMPLEMENTED_FUNC();
}

int VulkanMaterialSystem::AllocateDynamicLightmap(int lightmapSize[2], int* outOffsetIntoPage,
	int frameID)
{
	NOT_IMPLEMENTED_FUNC();
	return -1;
}

int VulkanMaterialSystem::GetNumSortIDs()
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void VulkanMaterialSystem::GetSortInfo(MaterialSystem_SortInfo_t* sortInfoArray)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ClearBuffers(bool clearColor, bool clearDepth, bool clearStencil)
{
	NOT_IMPLEMENTED_FUNC();
}

bool VulkanMaterialSystem::SupportsShadowDepthTextures()
{
	NOT_IMPLEMENTED_FUNC();
	return true;
}

ImageFormat VulkanMaterialSystem::GetShadowDepthTextureFormat()
{
	NOT_IMPLEMENTED_FUNC();
	return ImageFormat::IMAGE_FORMAT_UNKNOWN;
}

MaterialLock_t VulkanMaterialSystem::Lock()
{
	NOT_IMPLEMENTED_FUNC();
	return MaterialLock_t{};
}

void VulkanMaterialSystem::Unlock(MaterialLock_t lock)
{
	NOT_IMPLEMENTED_FUNC();
}

bool VulkanMaterialSystem::SupportsFetch4()
{
	NOT_IMPLEMENTED_FUNC();
	return true;
}

IMatRenderContext* VulkanMaterialSystem::GetRenderContext()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

IMatRenderContext* VulkanMaterialSystem::CreateRenderContext(MaterialContextType_t type)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

IMatRenderContext* VulkanMaterialSystem::SetRenderContext(IMatRenderContext* newCtx)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool VulkanMaterialSystem::SupportsCSAAMode(int numSamples, int qualityLevel)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

ImageFormat VulkanMaterialSystem::GetNullTextureFormat()
{
	return ImageFormat::IMAGE_FORMAT_UNKNOWN;
}

void VulkanMaterialSystem::AddTextureAlias(const char* alias, const char* realName)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::RemoveTextureAlias(const char* alias)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::SetExcludedTextures(const char* scriptName)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::UpdateExcludedTextures()
{
	NOT_IMPLEMENTED_FUNC();
}

bool VulkanMaterialSystem::IsInFrame() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void VulkanMaterialSystem::CompactMemory()
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::ReloadFilesInList(IFileList* filesToReload)
{
	NOT_IMPLEMENTED_FUNC();
}

bool VulkanMaterialSystem::AllowThreading(bool allow, int serviceThread)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void VulkanMaterialSystem::SetRenderTargetFrameBufferSizeOverrides(int width, int height)
{
	NOT_IMPLEMENTED_FUNC();
}

void VulkanMaterialSystem::GetRenderTargetFrameBufferDimensions(int& width, int& height)
{
	NOT_IMPLEMENTED_FUNC();
}

char* VulkanMaterialSystem::GetDisplayDeviceName() const
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

ITextureCompositor* VulkanMaterialSystem::NewTextureCompositor(int w, int h, const char* compositeName,
	int teamNum, uint64 randomSee, KeyValues* stageDesc, uint32_t texCompositeCreateFlags)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}
