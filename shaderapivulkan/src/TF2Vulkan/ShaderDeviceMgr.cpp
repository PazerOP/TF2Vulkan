#include "MaterialSystemHardwareConfig.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"

#include <stdshader_vulkan/ShaderBlobs.h>
#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/std_algorithm.h>

#include <tier0/icommandline.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>

#include <atomic>
#include <cstdlib>
#include <optional>
#include <vector>

using namespace TF2Vulkan;

namespace
{
	class ShaderDeviceMgr final : public CBaseAppSystem<IShaderDeviceMgrInternal>
	{
	public:
		InitReturnVal_t Init() override;
		bool Connect(CreateInterfaceFn factory) override;
		void* QueryInterface(const char* interfaceName) override;

		int GetAdapterCount() const override;
		void GetAdapterInfo(int adapter, MaterialAdapterInfo_t& info) const override;
		bool GetRecommendedConfigurationInfo(int adapter, int dxLevel, KeyValues* config) override;
		int GetModeCount(int adapter) const override;
		void GetModeInfo(ShaderDisplayMode_t* info, int adapter, int mode) const override;
		void GetCurrentModeInfo(ShaderDisplayMode_t* info, int adapter) const override;

		bool SetAdapter(int adapter, MaterialInitFlags_t flags) override;

		CreateInterfaceFn SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& mode) override;

		void AddModeChangeCallback(ShaderModeChangeCallbackFunc_t func) override;
		void RemoveModeChangeCallback(ShaderModeChangeCallbackFunc_t func) override;

		int GetAdapterIndex() const override;
		vk::PhysicalDevice GetAdapter() override;
		const vk::PhysicalDeviceProperties& GetAdapterProperties() const override { return m_AdapterProperties; }
		const vk::PhysicalDeviceFeatures& GetAdapterFeatures() const override { return m_AdapterFeatures; }

		const vk::Instance& GetInstance() const override;
		const vk::DispatchLoaderDynamic& GetDynamicDispatch() const override;

		const vk::AllocationCallbacks& GetAllocationCallbacks() const override;

	private:
		vk::PhysicalDevice GetAdapterByIndex(uint32_t index) const;

		std::vector<ShaderModeChangeCallbackFunc_t> m_ModeChangeCallbacks;

		struct DisplayAdapter
		{
			DISPLAY_DEVICEA m_WindowsInfo;
			std::vector<ShaderDisplayMode_t> m_Modes;
			ShaderDisplayMode_t GetCurrentMode() const;
		};
		std::vector<DisplayAdapter> m_Adapters;
		static std::vector<DisplayAdapter> LoadDisplayAdapters();

		vk::UniqueInstance m_Instance;
		vk::PhysicalDevice m_Adapter;
		vk::PhysicalDeviceProperties m_AdapterProperties;
		vk::PhysicalDeviceFeatures m_AdapterFeatures;

		vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> m_DebugMessenger;
		vk::DispatchLoaderDynamic m_InstanceDynDisp;

		bool m_HasBeenInit = false;
		int m_AdapterIndex = -1;
		MaterialInitFlags_t m_InitFlags{};
	};

	struct QueueFamily
	{
		uint32_t m_Index = uint32_t(-1);
		vk::QueueFamilyProperties m_Properties;
		float m_Priority = 1;

		vk::DeviceQueueCreateInfo ToQueueCreateInfo() const;
	};

	struct QueueFamilies
	{
		std::optional<QueueFamily> m_Graphics;
		std::optional<QueueFamily> m_Transfer;

		bool IsComplete() const { return m_Graphics.has_value(); }
		std::vector<vk::DeviceQueueCreateInfo> ToQueueCreateInfo() const;
	};
}

static ShaderDeviceMgr s_DeviceMgr;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShaderDeviceMgr, IShaderDeviceMgr, SHADER_DEVICE_MGR_INTERFACE_VERSION, s_DeviceMgr);

IShaderDeviceMgrInternal& TF2Vulkan::g_ShaderDeviceMgr = s_DeviceMgr;

static CDllDemandLoader s_StdShaderVulkanDll("stdshader_vulkan");
const IShaderBlobs* TF2Vulkan::g_ShaderBlobs;

static ConVar mat_alphacoverage("mat_alphacoverage", "0");
static ConVar mat_frame_sync_enable("mat_frame_sync_enable", "1");
static ConVar r_pixelfog("r_pixelfog", "1");
static ConVar mat_debugalttab("mat_debugalttab", "0");
//static ConVar mat_depthbias_shadowmap("mat_depthbias_shadowmap", "0");
//static ConVar mat_fastclip("mat_fastclip", "0");
//static ConVar mat_forcedynamic("mat_forcedynamic", "0");

static ConVar mat_hdr_level("mat_hdr_level", "2");
//static ConVar mat_slopescaledepthbias_shadowmap("mat_slopescaledepthbias_shadowmap", "0");
//static ConVar mat_supports_d3d9ex("mat_supports_d3d9ex", "0");
static ConVar mat_disable_ps_patch("mat_disable_ps_patch", "0");
static ConVar mat_force_ps_patch("mat_force_ps_patch", "0");
static ConVar mat_remoteshadercompile("mat_remoteshadercompile", "0");

static std::atomic_size_t s_ActiveAllocations = 0;
static std::array<std::array<std::atomic_size_t, VK_SYSTEM_ALLOCATION_SCOPE_RANGE_SIZE>, VK_INTERNAL_ALLOCATION_TYPE_RANGE_SIZE> s_ActiveAllocationsInternal = {};

static void* VKAPI_CALL AllocationFunction(void* userData, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	++s_ActiveAllocations;
	return _aligned_malloc(size, alignment);
}

static void* VKAPI_CALL ReallocationFunction(void* userData, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	if (!original)
		++s_ActiveAllocations;

	return _aligned_realloc(original, size, alignment);
}

static void VKAPI_CALL FreeFunction(void* userData, void* memory)
{
	--s_ActiveAllocations;
	return _aligned_free(memory);
}

static void VKAPI_CALL InternalAllocationNotificationFunction(void* userData, size_t size,
	VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
	// Do nothing
	++s_ActiveAllocationsInternal.at(allocationType).at(scope);
}

static void VKAPI_CALL InternalFreeNotificationFunction(void* userData, size_t size,
	VkInternalAllocationType allocationType, VkSystemAllocationScope scope)
{
	// Do nothing
	--s_ActiveAllocationsInternal.at(allocationType).at(scope);
}

static const vk::AllocationCallbacks s_AllocationCallbacks(
	nullptr, // User data
	&AllocationFunction,
	&ReallocationFunction,
	&FreeFunction,
	&InternalAllocationNotificationFunction,
	&InternalFreeNotificationFunction);

bool ShaderDeviceMgr::Connect(CreateInterfaceFn factory)
{
	LOG_FUNC();

	// MatSystemSurface normally adds 0.5 to vgui coordinates, unless
	// these command line parameters are present, in which case it
	// gets the offset from the params. We aren't DX9, so we don't
	// need the half-pixel offset.
	{
		// MYSTERY: AppendParm does nothing when we are using static linking???
		//CommandLine()->AppendParm("-pixel_offset_x", "0");
		//CommandLine()->AppendParm("-pixel_offset_y", "0");

		std::string oldCmdLine = CommandLine()->GetCmdLine();
		oldCmdLine += " -pixel_offset_x 0 -pixel_offset_y 0";
		CommandLine()->CreateCmdLine(oldCmdLine.c_str());
		const char* newCmdLine = CommandLine()->GetCmdLine();
		assert(oldCmdLine == newCmdLine);
	}

	ConnectTier1Libraries(&factory, 1);
	ConnectTier2Libraries(&factory, 1);
	ConnectTier3Libraries(&factory, 1);

	ConVar_Register();

	if (!CommandLine()->CheckParm("-insecure"))
	{
		Error("TF2Vulkan is likely to trigger VAC. For this reason, you MUST use -insecure"
			" on the command line (advanced options) to disable access to VAC-secured servers.");
	}

	// Interfaces from stdshader_vulkan
	{
		auto stdshaderVulkanFactory = s_StdShaderVulkanDll.GetFactory();
		Util::ConnectInterface(stdshaderVulkanFactory, SHADER_BLOBS_INTERFACE_VERSION, g_ShaderBlobs);
	}

	return true;
}

static inline auto tie(const ShaderDisplayMode_t& v)
{
	return std::tie(
		v.m_nVersion,
		v.m_nWidth,
		v.m_nHeight,
		v.m_Format,
		v.m_nRefreshRateNumerator,
		v.m_nRefreshRateDenominator);
}

static inline bool operator==(const ShaderDisplayMode_t& lhs, const ShaderDisplayMode_t& rhs)
{
	return tie(lhs) == tie(rhs);
}

static inline bool operator<(const ShaderDisplayMode_t& lhs, const ShaderDisplayMode_t& rhs)
{
	return tie(lhs) < tie(rhs);
}

static void ToDisplayMode(ShaderDisplayMode_t& output, const DEVMODEA& input)
{
	output.m_Format = IMAGE_FORMAT_UNKNOWN;
	Util::SafeConvert(input.dmPelsWidth, output.m_nWidth);
	Util::SafeConvert(input.dmPelsHeight, output.m_nHeight);
	Util::SafeConvert(input.dmDisplayFrequency, output.m_nRefreshRateNumerator);
	output.m_nRefreshRateDenominator = 1;
}

auto ShaderDeviceMgr::LoadDisplayAdapters() -> std::vector<DisplayAdapter>
{
	std::vector<DisplayAdapter> retVal;

	DISPLAY_DEVICEA inAdapter{};
	inAdapter.cb = sizeof(inAdapter);
	for (DWORD adapterIndex = 0; EnumDisplayDevicesA(nullptr, adapterIndex, &inAdapter, 0); adapterIndex++)
	{
		auto& newDispAdapter = retVal.emplace_back();
		newDispAdapter.m_WindowsInfo = inAdapter;

		// Load supported modes
		{
			auto& modes = newDispAdapter.m_Modes;

			DEVMODEA modeIn{};
			for (DWORD i = 0; EnumDisplaySettingsA(inAdapter.DeviceName, i, &modeIn); i++)
				ToDisplayMode(modes.emplace_back(), modeIn);

			std::sort(modes.begin(), modes.end());
			modes.erase(std::unique(modes.begin(), modes.end()), modes.end());
		}
	}

	return retVal;
}

ShaderDisplayMode_t ShaderDeviceMgr::DisplayAdapter::GetCurrentMode() const
{
	ShaderDisplayMode_t retVal;

	if (DEVMODEA mode; EnumDisplaySettingsA(m_WindowsInfo.DeviceName, ENUM_CURRENT_SETTINGS, &mode))
		ToDisplayMode(retVal, mode);

	return retVal;
}

static vk::UniqueInstance CreateInstance()
{
	vk::ApplicationInfo appInfo(
		"Team Fortress 2 (TF2Vulkan renderer)", VK_MAKE_VERSION(1, 0, 0),
		"Valve Source Engine (TF2 branch) (TF2Vulkan)", VK_MAKE_VERSION(1, 0, 0));

	vk::InstanceCreateInfo createInfo;
	createInfo.pApplicationInfo = &appInfo;

	constexpr const char* VALIDATION_LAYERS[] =
	{
		"VK_LAYER_LUNARG_standard_validation",
	};

	if (IsDebug() || CommandLine()->CheckParm("-tf2vk-validation"))
	{
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
		createInfo.enabledLayerCount = std::size(VALIDATION_LAYERS);
	}

	constexpr const char* INSTANCE_EXTENSIONS[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};
	createInfo.ppEnabledExtensionNames = INSTANCE_EXTENSIONS;
	createInfo.enabledExtensionCount = std::size(INSTANCE_EXTENSIONS);

	auto retVal = vk::createInstanceUnique(createInfo, s_AllocationCallbacks);
	if (!retVal)
		Error("[TF2Vulkan] Failed to create vulkan instance\n");

	return retVal;
}

vk::DeviceQueueCreateInfo QueueFamily::ToQueueCreateInfo() const
{
	vk::DeviceQueueCreateInfo retVal;

	retVal.queueFamilyIndex = m_Index;
	retVal.queueCount = 1;
	retVal.pQueuePriorities = &m_Priority;

	return retVal;
}

std::vector<vk::DeviceQueueCreateInfo> QueueFamilies::ToQueueCreateInfo() const
{
	if (!IsComplete())
	{
		assert(false);
		return {};
	}

	std::vector<vk::DeviceQueueCreateInfo> retVal;

	retVal.push_back(m_Graphics.value().ToQueueCreateInfo());

	if (m_Transfer)
		retVal.push_back(m_Transfer->ToQueueCreateInfo());

	return retVal;
}

static QueueFamilies FindQueueFamilies(const vk::PhysicalDevice& adapter)
{
	QueueFamilies retVal;

	auto propsList = adapter.getQueueFamilyProperties();

	uint32_t index = 0;
	for (const auto& props : propsList)
	{
		if (!retVal.m_Graphics &&
			(props.queueFlags & vk::QueueFlagBits::eGraphics) &&
			(props.queueCount > 0))
		{
			retVal.m_Graphics.emplace(QueueFamily{ index, props });
		}
		else if (!retVal.m_Transfer &&
			(props.queueFlags & vk::QueueFlagBits::eTransfer) &&
			(props.queueCount > 0))
		{
			retVal.m_Transfer.emplace(QueueFamily{ index, props });
		}

		index++;
	}

	return retVal;
}

static vk::UniqueDevice CreateDevice(vk::PhysicalDevice& adapter, QueueFamilies& queues)
{
	vk::DeviceCreateInfo createInfo;

	queues = FindQueueFamilies(adapter);
	if (!queues.IsComplete())
		throw VulkanException("Selected physical device does not support required queue families", EXCEPTION_DATA());

	auto queueCreateInfos = queues.ToQueueCreateInfo();

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();

	constexpr const char* DEVICE_EXTENSIONS[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;
	createInfo.enabledExtensionCount = std::size(DEVICE_EXTENSIONS);

	return adapter.createDeviceUnique(createInfo, s_AllocationCallbacks);
}

vk::PhysicalDevice ShaderDeviceMgr::GetAdapterByIndex(size_t index) const
{
	auto allAdapters = m_Instance->enumeratePhysicalDevices();
	if (index >= allAdapters.size())
	{
		assert(false);
		return 0;
	}

	return allAdapters[index];
}

static vk::Bool32 DebugUtilsMessengerCallback(
	const vk::DebugUtilsMessageSeverityFlagsEXT& severity,
	const vk::DebugUtilsMessageTypeFlagsEXT& types,
	const vk::DebugUtilsMessengerCallbackDataEXT& data)
{
	bool shouldBreak = false;
	auto msgFunc = &Msg;
	using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
	if (severity & (Severity::eError | Severity::eWarning))
	{
		msgFunc = &Warning;
		shouldBreak = !!(types & Type::eValidation);
	}

	msgFunc(
		"[TF2Vulkan] Debug Message:\n"
		"\tSeverity:   %s\n"
		"\tTypes:      %s\n"
		"\tMessage ID: %s (%i)\n"
		"\tMessage:    %s\n"
		"\n",
		vk::to_string(severity).c_str(),
		vk::to_string(types).c_str(),
		data.pMessageIdName, data.messageIdNumber,
		data.pMessage);

	if (shouldBreak)
		__debugbreak();

	return true;
}

InitReturnVal_t ShaderDeviceMgr::Init()
{
	LOG_FUNC();
	m_Instance = CreateInstance();

	m_InstanceDynDisp.init(m_Instance.get());

	// Debug messenger
	{
		vk::DebugUtilsMessengerCreateInfoEXT dbgCI;

		using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
		using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

		dbgCI.messageSeverity =
			Severity::eVerbose |
			//Severity::eInfo |
			Severity::eWarning |
			Severity::eError;

		dbgCI.messageType =
			Type::eGeneral |
			Type::ePerformance |
			Type::eValidation;

		dbgCI.pfnUserCallback = [](
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT * callbackData,
			void* userData) -> VkBool32
		{
			assert(callbackData);
			return (VkBool32)DebugUtilsMessengerCallback(
				(vk::DebugUtilsMessageSeverityFlagsEXT)messageSeverity,
				(vk::DebugUtilsMessageTypeFlagsEXT)messageTypes,
				*callbackData);
		};

		//m_DebugMessenger = m_Instance->createDebugUtilsMessengerEXTUnique(dbgCI, nullptr, m_InstanceDynDisp);
	}

	auto physicalDevices = m_Instance->enumeratePhysicalDevices();
	if (physicalDevices.size() < Util::SafeConvert<size_t>(m_AdapterIndex))
	{
		Warning("[TF2Vulkan] Adapter %i was selected, but only %zu adapters were found\n",
			m_AdapterIndex, physicalDevices.size());
		return InitReturnVal_t::INIT_FAILED;
	}

	m_Adapter = physicalDevices[m_AdapterIndex];
	m_AdapterProperties = m_Adapter.getProperties();
	m_AdapterFeatures = m_Adapter.getFeatures();

	QueueFamilies queueFamilies;
	if (auto device = CreateDevice(m_Adapter, queueFamilies))
	{
		IShaderDeviceInternal::VulkanInitData initData;
		initData.m_DeviceIndex = Util::SafeConvert<uint32_t>(m_AdapterIndex);
		initData.m_GraphicsQueueIndex = queueFamilies.m_Graphics.value().m_Index;

		if (queueFamilies.m_Transfer)
			initData.m_TransferQueueIndex = queueFamilies.m_Transfer->m_Index;

		initData.m_Device = std::move(device);
		g_ShaderDevice.VulkanInit(std::move(initData));
	}
	else
	{
		throw VulkanException("Failed to create vulkan device", EXCEPTION_DATA());
	}

	// Cache display modes
	m_Adapters = LoadDisplayAdapters();

	m_HasBeenInit = true;
	return InitReturnVal_t::INIT_OK;
}

void* ShaderDeviceMgr::QueryInterface(const char* interfaceName)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

int ShaderDeviceMgr::GetAdapterCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderDeviceMgr::GetAdapterInfo(int adapterIndex, MaterialAdapterInfo_t& info) const
{
	LOG_FUNC();
	auto adapter = GetAdapterByIndex(Util::SafeConvert<size_t>(adapterIndex));

	const auto props = adapter.getProperties();

	strcpy_s(info.m_pDriverName, props.deviceName);
	info.m_VendorID = props.vendorID;
	info.m_DeviceID = props.deviceID;
	info.m_SubSysID = 0;
	info.m_Revision = 1;
	info.m_nDXSupportLevel = 98;
	info.m_nMaxDXSupportLevel = 98;
	info.m_nDriverVersionHigh = props.apiVersion;
	info.m_nDriverVersionLow = props.driverVersion;
}

bool ShaderDeviceMgr::GetRecommendedConfigurationInfo(int adapter, int dxLevel, KeyValues* config)
{
	LOG_FUNC();
	return false; // We'll just let the user fend for themselves here (no auto config supported)
}

int ShaderDeviceMgr::GetModeCount(int adapterIndexSigned) const
{
	LOG_FUNC();

	// Vulkan doesn't really support exclusive fullscreen. Best we can do is borderless
	// windowed. In that case, just ask windows what the primary display's supported
	// modes are.
	return Util::SafeConvert<int>(m_Adapters.at(Util::SafeConvert<size_t>(adapterIndexSigned)).m_Modes.size());
}

void ShaderDeviceMgr::GetModeInfo(ShaderDisplayMode_t* info, int adapter, int mode) const
{
	LOG_FUNC();

	*info = m_Adapters.at(Util::SafeConvert<size_t>(adapter)).m_Modes.at(Util::SafeConvert<size_t>(mode));
}

void ShaderDeviceMgr::GetCurrentModeInfo(ShaderDisplayMode_t* info, int adapter) const
{
	LOG_FUNC();
	*info = m_Adapters.at(Util::SafeConvert<size_t>(adapter)).GetCurrentMode();
}

bool ShaderDeviceMgr::SetAdapter(int adapter, MaterialInitFlags_t flags)
{
	LOG_FUNC();
	if (m_HasBeenInit)
	{
		assert(false);
		return false;
	}

	if (adapter < 0)
	{
		assert(false);
		return false;
	}

	m_AdapterIndex = adapter;
	m_InitFlags = flags;
	return true;
}

CreateInterfaceFn ShaderDeviceMgr::SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& mode)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderDeviceMgr::AddModeChangeCallback(ShaderModeChangeCallbackFunc_t func)
{
	LOG_FUNC();
	assert(!Util::algorithm::contains(m_ModeChangeCallbacks, func));
	m_ModeChangeCallbacks.push_back(func);
}

void ShaderDeviceMgr::RemoveModeChangeCallback(ShaderModeChangeCallbackFunc_t func)
{
	LOG_FUNC();
	Util::algorithm::try_erase(m_ModeChangeCallbacks, func);
}

int ShaderDeviceMgr::GetAdapterIndex() const
{
	assert(m_HasBeenInit);
	if (!m_HasBeenInit)
		return -1;

	return m_AdapterIndex;
}

vk::PhysicalDevice ShaderDeviceMgr::GetAdapter()
{
	assert(m_Adapter);
	return m_Adapter;
}

const vk::Instance& ShaderDeviceMgr::GetInstance() const
{
	assert(m_Instance);
	return m_Instance.get();
}

const vk::DispatchLoaderDynamic& ShaderDeviceMgr::GetDynamicDispatch() const
{
	assert(m_Instance);
	return m_InstanceDynDisp;
}

const vk::AllocationCallbacks& ShaderDeviceMgr::GetAllocationCallbacks() const
{
	return s_AllocationCallbacks;
}
