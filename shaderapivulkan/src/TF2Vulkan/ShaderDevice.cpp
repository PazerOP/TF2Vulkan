#include "ShaderDevice.h"
#include "ShaderDeviceMgr.h"

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

		vk::Queue m_Queue;
		vk::UniqueCommandPool m_CommandPool;
	};

	class ShaderDevice final : public IShaderDeviceInternal
	{
	public:
		void ReleaseResources() override;
		void ReacquireResources() override;

		ImageFormat GetBackBufferFormat() const override;
		void GetBackBufferDimensions(int& width, int& height) const override;

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

		const IVulkanQueue& GetGraphicsQueue() override;
		Util::CheckedPtr<const IVulkanQueue> GetTransferQueue() override;

	private:
		// In a struct so we can easily reset all the vulkan-related stuff on shutdown
		struct VulkanData : VulkanInitData
		{
			VulkanData() = default;
			VulkanData(VulkanInitData&& base) : VulkanInitData(std::move(base)) {}

			vma::UniqueAllocator m_Allocator;

			VulkanQueue m_GraphicsQueue;
			std::optional<VulkanQueue> m_TransferQueue;

		} m_Data;
	};
}

static ShaderDevice s_Device;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(ShaderDevice, IShaderDevice, SHADER_DEVICE_INTERFACE_VERSION, s_Device);
IShaderDeviceInternal& TF2Vulkan::g_ShaderDevice = s_Device;

void ShaderDevice::ReleaseResources()
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::ReacquireResources()
{
	NOT_IMPLEMENTED_FUNC();
}

ImageFormat ShaderDevice::GetBackBufferFormat() const
{
	NOT_IMPLEMENTED_FUNC();
	return ImageFormat();
}

void ShaderDevice::GetBackBufferDimensions(int& width, int& height) const
{
	NOT_IMPLEMENTED_FUNC();
}

int ShaderDevice::GetCurrentAdapter() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(m_Data.m_DeviceIndex);
}

bool ShaderDevice::IsUsingGraphics() const
{
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
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::GetWindowSize(int& width, int& height) const
{
	NOT_IMPLEMENTED_FUNC();
}

void ShaderDevice::SetHardwareGammaRamp(float gamma, float gammaTVRangeMin, float gammaTVRangeMax, float gammaTVExponent, bool tvEnabled)
{
	NOT_IMPLEMENTED_FUNC();
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
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void ShaderDevice::DestroyStaticMesh(IMesh* mesh)
{
	NOT_IMPLEMENTED_FUNC();
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

	auto result = device.createCommandPoolUnique(createInfo);
	if (!result)
		Error(TF2VULKAN_PREFIX "Failed to create command pool for queue %u\n", queueFamily);

	return result;
}

static VulkanQueue CreateQueueWrapper(const vk::Device& device, uint32_t queueFamily)
{
	VulkanQueue retVal;

	retVal.m_Queue = device.getQueue(queueFamily, 0);
	if (!retVal.m_Queue)
		Error(TF2VULKAN_PREFIX "Failed to retrieve queue from index %u\n", queueFamily);

	retVal.m_CommandPool = CreateCommandPool(device, queueFamily);

	return retVal;
}

void ShaderDevice::VulkanInit(VulkanInitData&& inData)
{
	m_Data = std::move(inData);

	auto& device = m_Data.m_Device;
	m_Data.m_Allocator = CreateAllocator(device.get());

	m_Data.m_GraphicsQueue = CreateQueueWrapper(device.get(), m_Data.m_GraphicsQueueIndex);

	if (m_Data.m_TransferQueueIndex)
		m_Data.m_TransferQueue = CreateQueueWrapper(device.get(), m_Data.m_TransferQueueIndex.value());
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

const IVulkanQueue& ShaderDevice::GetGraphicsQueue()
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
