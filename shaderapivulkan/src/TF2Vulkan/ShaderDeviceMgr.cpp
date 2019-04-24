#include "Globals.h"
#include "IShaderDeviceMgrInternal.h"

#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/Placeholders.h>
#include <TF2Vulkan/Util/std_algorithm.h>

#include <vector>

#include <vulkan/vulkan.hpp>

using namespace TF2Vulkan;

namespace
{
	class ShaderDeviceMgr final : public CBaseAppSystem<IShaderDeviceMgrInternal>
	{
	public:
		InitReturnVal_t Init() override;
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

	private:
		std::vector<ShaderModeChangeCallbackFunc_t> m_ModeChangeCallbacks;

		vk::UniqueInstance m_Instance;
		vk::PhysicalDevice m_Adapter;

		bool m_HasBeenInit = false;
		int m_AdapterIndex = -1;
		MaterialInitFlags_t m_InitFlags{};
	};
}

static ShaderDeviceMgr s_DeviceMgr;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShaderDeviceMgr, IShaderDeviceMgr, SHADER_DEVICE_MGR_INTERFACE_VERSION, s_DeviceMgr);

IShaderDeviceMgrInternal& TF2Vulkan::g_ShaderDeviceMgr = s_DeviceMgr;

static vk::UniqueInstance CreateInstance()
{
	vk::ApplicationInfo appInfo(
		"Team Fortress 2 (TF2Vulkan renderer)", VK_MAKE_VERSION(1, 0, 0),
		"Valve Source Engine (TF2 branch) (TF2Vulkan)", VK_MAKE_VERSION(1, 0, 0));

	vk::InstanceCreateInfo createInfo;
	createInfo.pApplicationInfo = &appInfo;

	return vk::createInstanceUnique(createInfo);
}

InitReturnVal_t ShaderDeviceMgr::Init()
{
	LOG_FUNC();
	m_Instance = CreateInstance();
	if (!m_Instance)
	{
		Warning("[TF2Vulkan] Failed to create vulkan instance!\n");
		return InitReturnVal_t::INIT_FAILED;
	}

	auto physicalDevices = m_Instance->enumeratePhysicalDevices();
	if (physicalDevices.size() < size_t(m_AdapterIndex))
	{
		Warning("[TF2Vulkan] Adapter %i was selected, but only %zu adapters were found\n",
			m_AdapterIndex, physicalDevices.size());
		return InitReturnVal_t::INIT_FAILED;
	}

	m_Adapter = physicalDevices[m_AdapterIndex];

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

void ShaderDeviceMgr::GetAdapterInfo(int adapter, MaterialAdapterInfo_t& info) const
{
	NOT_IMPLEMENTED_FUNC();
}

bool ShaderDeviceMgr::GetRecommendedConfigurationInfo(int adapter, int dxLevel, KeyValues* config)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int ShaderDeviceMgr::GetModeCount(int adapter) const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void ShaderDeviceMgr::GetModeInfo(ShaderDisplayMode_t* info, int adapter, int mode) const
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDeviceMgr::GetCurrentModeInfo(ShaderDisplayMode_t* info, int adapter) const
{
	NOT_IMPLEMENTED_FUNC();
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
	if (!m_HasBeenInit)
	{
		assert(false);
		return nullptr;
	}

	return m_Adapter;
}
