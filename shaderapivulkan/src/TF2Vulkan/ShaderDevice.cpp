#include "FormatInfo.h"
#include "interface/internal/IShaderAPIInternal.h"
#include "interface/internal/IShaderAPITexture.h"
#include "IShaderTextureManager.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "ShaderDeviceMgr.h"
#include "VulkanCommandBufferBase.h"
#include "VulkanMesh.h"

#include <TF2Vulkan/Util/interface.h>

#pragma push_macro("min")
#pragma push_macro("max")

#undef min
#undef max
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

using namespace TF2Vulkan;

namespace
{
	struct VulkanQueue : public IVulkanQueue
	{
		const vk::Queue& GetQueue() const override { return m_Queue; }
		const vk::CommandPool& GetCmdPool() const override { return m_CommandPool.get(); }
		const vk::Device& GetDevice() const override;

		vk::Queue m_Queue;
		vk::UniqueCommandPool m_CommandPool;
	};

	struct VulkanSwapChain
	{
		void* m_HWND = nullptr;
		vk::UniqueSurfaceKHR m_Surface;
		vk::SwapchainCreateInfoKHR m_SwapChainCreateInfo;
		vk::UniqueSwapchainKHR m_SwapChain;

		struct PerImage
		{
			vk::Image m_Image;
			vk::UniqueImageView m_ImageView;

			vk::UniqueSemaphore m_ImageAvailableSemaphore;
			vk::UniqueSemaphore m_RenderFinishedSemaphore;
			vk::UniqueFence m_InFlightFence;

			std::unique_ptr<IVulkanCommandBuffer> m_PrimaryCmdBuf;
		};

		std::vector<PerImage> m_Images;

		uint32_t m_CurrentFrame = 0;
	};

	class ShaderDevice final : public IShaderDeviceInternal
	{
	public:
		void ReleaseResources() override;
		void ReacquireResources() override;

		ImageFormat GetBackBufferFormat() const override;
		void GetBackBufferDimensions(uint32_t& width, uint32_t& height) const override;

		int GetCurrentAdapter() const override;

		bool IsUsingGraphics() const override;

		void SpewDriverInfo() const override;

		int StencilBufferBits() const override;

		bool IsAAEnabled() const override;

		void Present() override;

		void GetWindowSize(int& width, int& height) const override;

		void SetHardwareGammaRamp(float gamma, float gammaTVRangeMin, float gammaTVRangeMax,
			float gammaTVExponent, bool tvEnabled) override;

		bool AddView(void* hwnd) override;
		void RemoveView(void* hwnd) override;

		void SetView(void* hwnd) override;

		IShaderBuffer* CompileShader(const char* program, size_t bufLen, const char* shaderVersion) override;

		VertexShaderHandle_t CreateVertexShader(IShaderBuffer* buf) override;
		void DestroyVertexShader(VertexShaderHandle_t shader) override;
		GeometryShaderHandle_t CreateGeometryShader(IShaderBuffer* buf) override;
		void DestroyGeometryShader(GeometryShaderHandle_t shader) override;
		PixelShaderHandle_t CreatePixelShader(IShaderBuffer* buf) override;
		void DestroyPixelShader(PixelShaderHandle_t shader) override;

		IMesh* CreateStaticMesh(VertexFormat_t format, const char* textureBudgetGroup, IMaterial* material) override;
		void DestroyStaticMesh(IMesh* mesh) override;

		IVertexBuffer* CreateVertexBuffer(ShaderBufferType_t type, VertexFormat_t fmt, int vertexCount,
			const char* budgetGroup) override;
		void DestroyVertexBuffer(IVertexBuffer* buffer) override;
		IIndexBuffer* CreateIndexBuffer(ShaderBufferType_t type, MaterialIndexFormat_t fmt, int indexCount,
			const char* budgetGroup) override;
		void DestroyIndexBuffer(IIndexBuffer* buffer) override;

		IVertexBuffer* GetDynamicVertexBuffer(int streamID, VertexFormat_t format, bool buffered) override;
		IIndexBuffer* GetDynamicIndexBuffer(MaterialIndexFormat_t fmt, bool buffered) override;

		void EnableNonInteractiveMode(MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t* info) override;
		void RefreshFrontBufferNonInteractive() override;
		void HandleThreadEvent(uint32 threadEvent) override;

		char* GetDisplayDeviceName() override;

		void VulkanInit(VulkanInitData&& device) override;
		const vk::Device& GetVulkanDevice() override;

		vma::UniqueAllocator& GetVulkanAllocator() override;

		IVulkanQueue& GetGraphicsQueue() override;
		Util::CheckedPtr<const IVulkanQueue> GetTransferQueue() override;

		bool SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& info) override;

		using IShaderDeviceInternal::SetDebugName;
		void SetDebugName(uint64_t obj, vk::ObjectType type, const char* name) override;

		const IShaderAPITexture& GetBackBufferColorTexture() const override { return m_BackbufferColorTexture; }
		const IShaderAPITexture& GetBackBufferDepthTexture() const override;

		bool IsReady() const override;
		IVulkanCommandBuffer& GetPrimaryCmdBuf() override;
		const vk::DispatchLoaderDynamic& GetDynamicDispatch() const override { return m_Data.m_DynamicLoader; }

	private:
		// In a struct so we can easily reset all the vulkan-related stuff on shutdown
		struct VulkanData : VulkanInitData
		{
			VulkanData() = default;
			VulkanData(VulkanInitData&& base) : VulkanInitData(std::move(base)) {}

			vma::UniqueAllocator m_Allocator;

			VulkanQueue m_GraphicsQueue;
			std::optional<VulkanQueue> m_TransferQueue;
			VulkanSwapChain m_SwapChain;
			vk::DispatchLoaderDynamic m_DynamicLoader;

			std::unique_ptr<IVulkanCommandBuffer> m_TempPrimaryCmdBuf;

			const IShaderAPITexture* m_DepthTexture = nullptr;

		} m_Data;

		struct BackbufferColorTexture : IShaderAPITexture
		{
			std::string_view GetDebugName() const override { return "__rt_tf2vulkan_backbuffer"; }
			const vk::Image& GetImage() const override;
			const vk::ImageCreateInfo& GetImageCreateInfo() const override;
			const vk::ImageView& FindOrCreateView() override;
			const vk::ImageView& FindOrCreateView(const vk::ImageViewCreateInfo& createInfo) override { NOT_IMPLEMENTED_FUNC(); }
			ShaderAPITextureHandle_t GetHandle() const override { return 0; }
		};
		BackbufferColorTexture m_BackbufferColorTexture;
	};
}

static ShaderDevice s_Device;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShaderDevice, IShaderDevice, SHADER_DEVICE_INTERFACE_VERSION, s_Device);
IShaderDeviceInternal& TF2Vulkan::g_ShaderDevice = s_Device;

void ShaderDevice::ReleaseResources()
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void ShaderDevice::ReacquireResources()
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

ImageFormat ShaderDevice::GetBackBufferFormat() const
{
	LOG_FUNC();
	return FormatInfo::ConvertImageFormat(m_Data.m_SwapChain.m_SwapChainCreateInfo.imageFormat);
}

void ShaderDevice::GetBackBufferDimensions(uint32_t& width, uint32_t& height) const
{
	LOG_FUNC();
	auto& swapChainExtent = m_Data.m_SwapChain.m_SwapChainCreateInfo.imageExtent;

	width = swapChainExtent.width;
	height = swapChainExtent.height;
}

int ShaderDevice::GetCurrentAdapter() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(m_Data.m_DeviceIndex);
}

bool ShaderDevice::IsUsingGraphics() const
{
	LOG_FUNC();
	return true;
}

void ShaderDevice::SpewDriverInfo() const
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderDevice::StencilBufferBits() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool ShaderDevice::IsAAEnabled() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderDevice::Present()
{
	LOG_FUNC();

	// TODO: What is this function actually even doing
	auto& scData = m_Data.m_SwapChain;
	auto& sc = scData.m_SwapChain.get();
	auto& currentFrame = scData.m_CurrentFrame;
	auto& curImg = scData.m_Images.at(currentFrame);
	auto& device = GetVulkanDevice();

	auto pixScope = curImg.m_PrimaryCmdBuf->DebugRegionBegin("ShaderDevice::Present()");

	constexpr auto timeout = std::numeric_limits<uint32_t>::max();

	device.waitForFences(curImg.m_InFlightFence.get(), true, timeout);
	device.resetFences(curImg.m_InFlightFence.get());

	const auto [acquireResult, acquireImageIndex] = device.acquireNextImageKHR(sc,
		std::numeric_limits<uint64_t>::max(), curImg.m_ImageAvailableSemaphore.get(),
		nullptr);

#pragma warning(suppress : 4996)
	auto& primaryCmdBuf = *curImg.m_PrimaryCmdBuf;
	primaryCmdBuf.endRenderPass();

	// Prepare swapchain for presentation
	TransitionImageLayout(curImg.m_Image, scData.m_SwapChainCreateInfo.imageFormat,
		vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
		primaryCmdBuf, 0);

	primaryCmdBuf.end();

	auto& q = m_Data.m_GraphicsQueue.m_Queue;
	{
		vk::SubmitInfo submitInfo;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &curImg.m_ImageAvailableSemaphore.get();

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &curImg.m_RenderFinishedSemaphore.get();

		const vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submitInfo.pWaitDstStageMask = &waitStages;

		primaryCmdBuf.Submit(submitInfo, curImg.m_InFlightFence.get());

		device.waitForFences(curImg.m_InFlightFence.get(), true, timeout);
	}

	// Present
	{
		vk::PresentInfoKHR pInfo;

		pInfo.pWaitSemaphores = &curImg.m_RenderFinishedSemaphore.get();
		pInfo.waitSemaphoreCount = 1;

		pInfo.pSwapchains = &sc;
		pInfo.swapchainCount = 1;

		pInfo.pImageIndices = &currentFrame;

		q.presentKHR(pInfo);

		q.waitIdle();
	}

	// Reset the command buffer
	{
		primaryCmdBuf.reset();

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		primaryCmdBuf.begin(beginInfo);

		TransitionImageLayout(curImg.m_Image, scData.m_SwapChainCreateInfo.imageFormat,
			vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal,
			primaryCmdBuf, 0);
	}

	currentFrame = (currentFrame + 1) % scData.m_Images.size();
}

void ShaderDevice::GetWindowSize(int& width, int& height) const
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::SetHardwareGammaRamp(float gamma, float gammaTVRangeMin, float gammaTVRangeMax, float gammaTVExponent, bool tvEnabled)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

bool ShaderDevice::AddView(void* hwnd)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void ShaderDevice::RemoveView(void* hwnd)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::SetView(void* hwnd)
{
	NOT_IMPLEMENTED_FUNC();
}

IShaderBuffer* ShaderDevice::CompileShader(const char* program, size_t bufLen, const char* shaderVersion)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

VertexShaderHandle_t ShaderDevice::CreateVertexShader(IShaderBuffer* buf)
{
	NOT_IMPLEMENTED_FUNC();
	return VertexShaderHandle_t();
}

void ShaderDevice::DestroyVertexShader(VertexShaderHandle_t shader)
{
	NOT_IMPLEMENTED_FUNC();
}

GeometryShaderHandle_t ShaderDevice::CreateGeometryShader(IShaderBuffer* buf)
{
	NOT_IMPLEMENTED_FUNC();
	return GeometryShaderHandle_t();
}

void ShaderDevice::DestroyGeometryShader(GeometryShaderHandle_t shader)
{
	NOT_IMPLEMENTED_FUNC();
}

PixelShaderHandle_t ShaderDevice::CreatePixelShader(IShaderBuffer* buf)
{
	NOT_IMPLEMENTED_FUNC();
	return PixelShaderHandle_t();
}

void ShaderDevice::DestroyPixelShader(PixelShaderHandle_t shader)
{
	NOT_IMPLEMENTED_FUNC();
}

IMesh* ShaderDevice::CreateStaticMesh(VertexFormat_t format, const char* textureBudgetGroup, IMaterial* material)
{
	// TODO
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	return new VulkanMesh(VertexFormat(format));
}

void ShaderDevice::DestroyStaticMesh(IMesh* mesh)
{
	// TODO
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	delete assert_cast<VulkanMesh*>(mesh);
}

IVertexBuffer* ShaderDevice::CreateVertexBuffer(ShaderBufferType_t type, VertexFormat_t fmt, int vertexCount, const char* budgetGroup)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderDevice::DestroyVertexBuffer(IVertexBuffer* buffer)
{
	NOT_IMPLEMENTED_FUNC();
}

IIndexBuffer* ShaderDevice::CreateIndexBuffer(ShaderBufferType_t type, MaterialIndexFormat_t fmt, int indexCount, const char* budgetGroup)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderDevice::DestroyIndexBuffer(IIndexBuffer* buffer)
{
	NOT_IMPLEMENTED_FUNC();
}

IVertexBuffer* ShaderDevice::GetDynamicVertexBuffer(int streamID, VertexFormat_t format, bool buffered)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

IIndexBuffer* ShaderDevice::GetDynamicIndexBuffer(MaterialIndexFormat_t fmt, bool buffered)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderDevice::EnableNonInteractiveMode(MaterialNonInteractiveMode_t mode, ShaderNonInteractiveInfo_t* info)
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::RefreshFrontBufferNonInteractive()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::HandleThreadEvent(uint32 threadEvent)
{
	NOT_IMPLEMENTED_FUNC();
}

char* ShaderDevice::GetDisplayDeviceName()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

static vma::UniqueAllocator CreateAllocator(vk::Device& device)
{
	vma::AllocatorCreateInfo info;
	info.device = (VkDevice)device;
	info.physicalDevice = (VkPhysicalDevice)g_ShaderDeviceMgr.GetAdapter();

	return vma::createAllocatorUnique(info);
}

static vk::UniqueCommandPool CreateCommandPool(const vk::Device& device, uint32_t queueFamily)
{
	vk::CommandPoolCreateInfo createInfo({}, queueFamily);

	// Allow resetting command buffers
	createInfo.flags |= vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	auto result = device.createCommandPoolUnique(createInfo);
	if (!result)
		Error(TF2VULKAN_PREFIX "Failed to create command pool for queue %u\n", queueFamily);

	return result;
}

static VulkanQueue CreateQueueWrapper(const vk::Device& device, uint32_t queueFamily, const char* queueType)
{
	VulkanQueue retVal;

	retVal.m_Queue = device.getQueue(queueFamily, 0);
	if (!retVal.m_Queue)
		Error(TF2VULKAN_PREFIX "Failed to retrieve %s queue from index %u\n", queueType, queueFamily);

	retVal.m_CommandPool = CreateCommandPool(device, queueFamily);

	char buf[128];
	sprintf_s(buf, "TF2Vulkan Queue (%s)", queueType);
	g_ShaderDevice.SetDebugName(retVal.m_Queue, buf);

	sprintf_s(buf, "TF2Vulkan Queue Command Pool (%s)", queueType);
	g_ShaderDevice.SetDebugName(retVal.m_CommandPool, buf);

	return retVal;
}

void ShaderDevice::VulkanInit(VulkanInitData&& inData)
{
	m_Data = std::move(inData);

	auto& device = m_Data.m_Device;
	m_Data.m_Allocator = CreateAllocator(device.get());

	m_Data.m_DynamicLoader.init(g_ShaderDeviceMgr.GetInstance(), m_Data.m_Device.get());

	SetDebugName(m_Data.m_Device, "TF2Vulkan Device");

	m_Data.m_GraphicsQueue = CreateQueueWrapper(device.get(), m_Data.m_GraphicsQueueIndex, "Graphics");

	// So GetPrimaryCmdBuf() gives something safe
	m_Data.m_TempPrimaryCmdBuf = m_Data.m_GraphicsQueue.CreateCmdBufferAndBegin();

	if (m_Data.m_TransferQueueIndex)
		m_Data.m_TransferQueue = CreateQueueWrapper(device.get(), m_Data.m_TransferQueueIndex.value(), "Transfer");
}

const vk::Device& ShaderDevice::GetVulkanDevice()
{
	assert(m_Data.m_Device);
	return m_Data.m_Device.get();
}

vma::UniqueAllocator& ShaderDevice::GetVulkanAllocator()
{
	assert(m_Data.m_Allocator);
	return m_Data.m_Allocator;
}

IVulkanQueue& ShaderDevice::GetGraphicsQueue()
{
	assert(m_Data.m_GraphicsQueue.m_Queue);
	assert(m_Data.m_GraphicsQueue.m_CommandPool);
	return m_Data.m_GraphicsQueue;
}

Util::CheckedPtr<const IVulkanQueue> ShaderDevice::GetTransferQueue()
{
	assert(m_Data.m_Device);
	auto& queue = m_Data.m_TransferQueue;
	return queue ? &*queue : nullptr;
}

bool ShaderDevice::SetMode(void* hwnd, int adapter, const ShaderDeviceInfo_t& info)
{
	LOG_FUNC();

	VulkanSwapChain newSwapChain;
	newSwapChain.m_HWND = hwnd;

	// Window surface
	{
		vk::Win32SurfaceCreateInfoKHR surfCreateInfo;
		surfCreateInfo.hwnd = HWND(hwnd);
		surfCreateInfo.hinstance = HINSTANCE(GetWindowLong(surfCreateInfo.hwnd, GWL_HINSTANCE));

		newSwapChain.m_Surface = g_ShaderDeviceMgr.GetInstance().createWin32SurfaceKHRUnique(surfCreateInfo);
		if (!newSwapChain.m_Surface)
			throw VulkanException("Failed to create window surface", EXCEPTION_DATA());
	}

	// Swapchain
	{
		auto& scCreateInfo = newSwapChain.m_SwapChainCreateInfo;

		const auto adapter = g_ShaderDeviceMgr.GetAdapter();
		const auto surfCaps = adapter.getSurfaceCapabilitiesKHR(newSwapChain.m_Surface.get());
		const auto surfFmts = adapter.getSurfaceFormatsKHR(newSwapChain.m_Surface.get());
		const auto presentModes = adapter.getSurfacePresentModesKHR(newSwapChain.m_Surface.get());

		bool supported = adapter.getSurfaceSupportKHR(m_Data.m_GraphicsQueueIndex, newSwapChain.m_Surface.get());

		scCreateInfo.surface = newSwapChain.m_Surface.get();
		scCreateInfo.minImageCount = surfCaps.minImageCount;
		scCreateInfo.imageFormat = surfFmts.at(0).format;
		scCreateInfo.imageColorSpace = surfFmts.at(0).colorSpace;
		scCreateInfo.imageExtent = surfCaps.currentExtent;
		scCreateInfo.imageArrayLayers = 1;
		scCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		scCreateInfo.queueFamilyIndexCount = 1;
		scCreateInfo.pQueueFamilyIndices = &m_Data.m_GraphicsQueueIndex;
		scCreateInfo.presentMode = presentModes.at(0);

		newSwapChain.m_SwapChain = m_Data.m_Device->createSwapchainKHRUnique(scCreateInfo);
		SetDebugName(newSwapChain.m_SwapChain, "TF2Vulkan Swap Chain");
	}

	// Swap chain images and image views
	{
		const auto images = m_Data.m_Device->getSwapchainImagesKHR(newSwapChain.m_SwapChain.get());

		size_t index = 0;
		for (auto& img : images)
		{
			auto& perImg = newSwapChain.m_Images.emplace_back();
			perImg.m_Image = img;

			auto& scCI = newSwapChain.m_SwapChainCreateInfo;
			vk::ImageViewCreateInfo ci;

			ci.format = scCI.imageFormat;
			ci.subresourceRange.layerCount = 1;
			ci.subresourceRange.levelCount = 1;
			ci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			ci.viewType = vk::ImageViewType::e2D;

			ci.image = img;
			perImg.m_ImageView = m_Data.m_Device->createImageViewUnique(ci);

			char buf[128];
			sprintf_s(buf, "TF2Vulkan Swap Chain Image View #%zu", index);
			SetDebugName(perImg.m_ImageView, buf);
			sprintf_s(buf, "TF2Vulkan Swap Chain Image #%zu", index);
			SetDebugName(img, buf);

			perImg.m_PrimaryCmdBuf = GetGraphicsQueue().CreateCmdBufferAndBegin();

			// Initially, images start in ePresentSrcKHR layout. We want them
			// to be in eColorAttachmentOptimal, since that's what ::Present()
			// is expecting, since that's what ::Present() leaves the swapchain
			// images in after presentation.
			TransitionImageLayout(img, ci.format,
				vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal,
				*perImg.m_PrimaryCmdBuf, 0);

			index++;
		}
	}

	// Synchronization objects
	{
		vk::SemaphoreCreateInfo sCI;

		vk::FenceCreateInfo fCI;
		fCI.flags = vk::FenceCreateFlagBits::eSignaled;

		size_t index = 0;
		for (auto& img : newSwapChain.m_Images)
		{
			char buf[128];
			img.m_ImageAvailableSemaphore = m_Data.m_Device->createSemaphoreUnique(sCI);
			sprintf_s(buf, "TF2Vulkan Image Available Semaphore #%zu", index);
			SetDebugName(img.m_ImageAvailableSemaphore, buf);

			img.m_RenderFinishedSemaphore = m_Data.m_Device->createSemaphoreUnique(sCI);
			sprintf_s(buf, "TF2Vulkan Render Finished Semaphore #%zu", index);
			SetDebugName(img.m_RenderFinishedSemaphore, buf);

			img.m_InFlightFence = m_Data.m_Device->createFenceUnique(fCI);
			sprintf_s(buf, "TF2Vulkan In Flight Fence #%zu", index);
			SetDebugName(img.m_InFlightFence, buf);

			index++;
		}
	}

	// Depth buffer
	{
		vk::ImageCreateInfo ci;
		ci.format = FormatInfo::PromoteToHardware(vk::Format::eD24UnormS8Uint, FormatUsage::DepthStencil, false);
		ci.extent.width = newSwapChain.m_SwapChainCreateInfo.imageExtent.width;
		ci.extent.height = newSwapChain.m_SwapChainCreateInfo.imageExtent.height;
		ci.extent.depth = 1;
		ci.arrayLayers = 1;
		ci.imageType = vk::ImageType::e2D;
		ci.mipLevels = 1;
		ci.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;

		assert(!m_Data.m_DepthTexture); // TODO
		m_Data.m_DepthTexture = &g_TextureManager.CreateTexture("__rt_tf2vulkan_depth", ci);
	}

	m_Data.m_SwapChain = std::move(newSwapChain);

	if (m_Data.m_TempPrimaryCmdBuf)
	{
		m_Data.m_TempPrimaryCmdBuf->Submit();
		m_Data.m_TempPrimaryCmdBuf.reset();
	}

	return true;
}

const vk::Device& VulkanQueue::GetDevice() const
{
	return s_Device.GetVulkanDevice();
}

void ShaderDevice::SetDebugName(uint64_t obj, vk::ObjectType type, const char* name)
{
	vk::DebugUtilsObjectNameInfoEXT info;
	info.objectHandle = obj;
	info.objectType = type;
	info.pObjectName = name;

	GetVulkanDevice().setDebugUtilsObjectNameEXT(info, m_Data.m_DynamicLoader);
}

const vk::Image& ShaderDevice::BackbufferColorTexture::GetImage() const
{
	auto& sc = s_Device.m_Data.m_SwapChain;
	return sc.m_Images.at(sc.m_CurrentFrame).m_Image;
}

const vk::ImageView& ShaderDevice::BackbufferColorTexture::FindOrCreateView()
{
	auto& sc = s_Device.m_Data.m_SwapChain;
	return sc.m_Images.at(sc.m_CurrentFrame).m_ImageView.get();
}

const vk::ImageCreateInfo& ShaderDevice::BackbufferColorTexture::GetImageCreateInfo() const
{
	// FIXME REALLY SOON
	static thread_local vk::ImageCreateInfo s_BBIC;

	const auto& scci = s_Device.m_Data.m_SwapChain.m_SwapChainCreateInfo;

	s_BBIC.format = scci.imageFormat;
	s_BBIC.extent.width = scci.imageExtent.width;
	s_BBIC.extent.height = scci.imageExtent.height;
	s_BBIC.extent.depth = 1;
	s_BBIC.mipLevels = 1;
	s_BBIC.usage = scci.imageUsage;
	s_BBIC.arrayLayers = scci.imageArrayLayers;
	s_BBIC.sharingMode = scci.imageSharingMode;

	return s_BBIC;
}

const IShaderAPITexture& ShaderDevice::GetBackBufferDepthTexture() const
{
	assert(IsReady());

	const auto& tex = m_Data.m_DepthTexture;
	assert(tex);
	return *tex;
}

bool ShaderDevice::IsReady() const
{
	// TODO: Is this a safe way to check we've been initialized?
	return !!m_Data.m_DepthTexture;
}

IVulkanCommandBuffer& ShaderDevice::GetPrimaryCmdBuf()
{
	auto& scData = m_Data.m_SwapChain;

	if (scData.m_Images.size() > scData.m_CurrentFrame)
		return *scData.m_Images[scData.m_CurrentFrame].m_PrimaryCmdBuf;
	else
		return *m_Data.m_TempPrimaryCmdBuf;
}
