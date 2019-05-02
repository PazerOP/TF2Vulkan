#include "MaterialSystemHardwareConfig.h"
#include "ShaderDeviceMgr.h"
#include "ShaderDevice.h"

#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/std_algorithm.h>

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

		vk::Instance GetInstance() override;

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
	createInfo.ppEnabledLayerNames = VALIDATION_LAYERS;
	createInfo.enabledLayerCount = std::size(VALIDATION_LAYERS);

	constexpr const char* INSTANCE_EXTENSIONS[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};
	createInfo.ppEnabledExtensionNames = INSTANCE_EXTENSIONS;
	createInfo.enabledExtensionCount = std::size(INSTANCE_EXTENSIONS);

	auto retVal = vk::createInstanceUnique(createInfo);
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

	return adapter.createDeviceUnique(createInfo);
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

bool ShaderDeviceMgr::Connect(CreateInterfaceFn factory)
{
	LOG_FUNC();
	return true;
}

InitReturnVal_t ShaderDeviceMgr::Init()
{
	LOG_FUNC();
	m_Instance = CreateInstance();

	auto physicalDevices = m_Instance->enumeratePhysicalDevices();
	if (physicalDevices.size() < Util::SafeConvert<size_t>(m_AdapterIndex))
	{
		Warning("[TF2Vulkan] Adapter %i was selected, but only %zu adapters were found\n",
			m_AdapterIndex, physicalDevices.size());
		return InitReturnVal_t::INIT_FAILED;
	}

	m_Adapter = physicalDevices[m_AdapterIndex];
	g_MatSysConfig.Init();

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

vk::Instance ShaderDeviceMgr::GetInstance()
{
	assert(m_Instance);
	return m_Instance.get();
}
