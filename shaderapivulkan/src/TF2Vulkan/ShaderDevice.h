#pragma once

#include "interface/internal/IVulkanCommandBuffer.h"
#include "interface/internal/IVulkanQueue.h"
#include <TF2Vulkan/Util/Checked.h>

#include <shaderapi/IShaderDevice.h>

#include <optional>

namespace TF2Vulkan
{
	class IShaderAPITexture;
	class IVulkanTexture;

#pragma push_macro("SET_DEBUG_NAME_FN")
#undef SET_DEBUG_NAME_FN
#define SET_DEBUG_NAME_FN(type) \
	inline void SetDebugName(const vk:: ## type & obj, const char* name) \
	{ \
		return SetDebugName((uint64_t)(Vk ## type)obj, vk::ObjectType::e ## type, name); \
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
		virtual const vk::DispatchLoaderDynamic& GetDynamicDispatch() const = 0;

		virtual IVulkanQueue& GetGraphicsQueue() = 0;

		virtual Util::CheckedPtr<const IVulkanQueue> GetTransferQueue() = 0;

		virtual const IShaderAPITexture& GetBackBufferColorTexture() const = 0;
		IShaderAPITexture& GetBackBufferColorTexture()
		{
			return const_cast<IShaderAPITexture&>(std::as_const(*this).GetBackBufferColorTexture());
		}
		virtual const IShaderAPITexture& GetBackBufferDepthTexture() const = 0;
		IShaderAPITexture& GetBackBufferDepthTexture()
		{
			return const_cast<IShaderAPITexture&>(std::as_const(*this).GetBackBufferDepthTexture());
		}

		virtual IVulkanCommandBuffer& GetPrimaryCmdBuf() = 0;

		virtual bool SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& info) = 0;

		SET_DEBUG_NAME_FN(Buffer);
		SET_DEBUG_NAME_FN(CommandBuffer);
		SET_DEBUG_NAME_FN(CommandPool);
		SET_DEBUG_NAME_FN(Device);
		SET_DEBUG_NAME_FN(DescriptorPool);
		SET_DEBUG_NAME_FN(DescriptorSetLayout);
		SET_DEBUG_NAME_FN(Fence);
		SET_DEBUG_NAME_FN(Framebuffer);
		SET_DEBUG_NAME_FN(Image);
		SET_DEBUG_NAME_FN(ImageView);
		SET_DEBUG_NAME_FN(Pipeline);
		SET_DEBUG_NAME_FN(PipelineLayout);
		SET_DEBUG_NAME_FN(Queue);
		SET_DEBUG_NAME_FN(RenderPass);
		SET_DEBUG_NAME_FN(Sampler);
		SET_DEBUG_NAME_FN(Semaphore);
		SET_DEBUG_NAME_FN(ShaderModule);
		SET_DEBUG_NAME_FN(SwapchainKHR);
		template<typename Type, typename Dispatch>
		void SetDebugName(const vk::UniqueHandle<Type, Dispatch>& obj, const char* name)
		{
			return SetDebugName(obj.get(), name);
		}

		virtual void SetDebugName(uint64_t obj, vk::ObjectType type, const char* name) = 0;

		virtual void GetBackBufferDimensions(uint32_t& width, uint32_t& height) const = 0;
		void GetBackBufferDimensions(int& width, int& height) const override final
		{
			LOG_FUNC();
			uint32_t uwidth, uheight;
			GetBackBufferDimensions(uwidth, uheight);
			Util::SafeConvert(uwidth, width);
			Util::SafeConvert(uheight, height);
		}

		virtual bool IsReady() const = 0;
	};

	extern IShaderDeviceInternal& g_ShaderDevice;

#pragma pop_macro("SET_DEBUG_NAME_FN")
}
