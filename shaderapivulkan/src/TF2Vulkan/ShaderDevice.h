#pragma once

#include <TF2Vulkan/IVulkanQueue.h>
#include <TF2Vulkan/Util/Checked.h>

#include <shaderapi/IShaderDevice.h>

#include <optional>

namespace TF2Vulkan
{
	class IShaderShadowInternal;

#pragma push_macro("SET_DEBUG_NAME_FN")
#undef SET_DEBUG_NAME_FN
#define SET_DEBUG_NAME_FN(type) \
	inline void SetDebugName(const vk:: ## type & obj, const char* name) \
	{ \
		return SetDebugName((Vk ## type)obj, vk::ObjectType::e ## type, name); \
	}

	class IShaderDeviceInternal : public IShaderDevice
	{
	public:
		struct VulkanInitData
		{
			uint32_t m_DeviceIndex = uint32_t(-1);
			vk::UniqueDevice m_Device;
			uint32_t m_GraphicsQueueIndex = uint32_t(-1);
			std::optional<uint32_t> m_TransferQueueIndex;
		};

		virtual void VulkanInit(VulkanInitData&& data) = 0;

		virtual const vk::Device& GetVulkanDevice() = 0;
		virtual vma::UniqueAllocator& GetVulkanAllocator() = 0;

		virtual const IVulkanQueue& GetGraphicsQueue() = 0;

		virtual Util::CheckedPtr<const IVulkanQueue> GetTransferQueue() = 0;

		virtual bool SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& info) = 0;

		SET_DEBUG_NAME_FN(DescriptorSetLayout);
		SET_DEBUG_NAME_FN(Pipeline);
		SET_DEBUG_NAME_FN(PipelineLayout);
		SET_DEBUG_NAME_FN(RenderPass);
		SET_DEBUG_NAME_FN(ShaderModule);
		template<typename Type, typename Dispatch>
		void SetDebugName(const vk::UniqueHandle<Type, Dispatch>& obj, const char* name)
		{
			return SetDebugName(obj.get(), name);
		}

		virtual void SetDebugName(uint64_t obj, vk::ObjectType type, const char* name) = 0;
	};

	extern IShaderDeviceInternal& g_ShaderDevice;

#pragma pop_macro("SET_DEBUG_NAME_FN")
}
