#pragma once

#include <shaderapi/IShaderDevice.h>

enum MaterialInitFlags_t;

namespace TF2Vulkan
{
	class IShaderDeviceMgrInternal : public IShaderDeviceMgr
	{
	public:
		virtual int GetAdapterIndex() const = 0;

		virtual bool SetAdapter(int adapter, MaterialInitFlags_t flags) = 0;
		bool SetAdapter(int adapter, int flags) override final
		{
			return SetAdapter(adapter, MaterialInitFlags_t(flags));
		}

		virtual vk::PhysicalDevice GetAdapter() = 0;

		virtual const vk::Instance& GetInstance() const = 0;
		virtual const vk::DispatchLoaderDynamic& GetDynamicDispatch() const = 0;
	};

	extern IShaderDeviceMgrInternal& g_ShaderDeviceMgr;
}
