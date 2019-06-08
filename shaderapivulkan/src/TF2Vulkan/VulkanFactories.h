#pragma once

namespace TF2Vulkan
{
	class IVulkanTexture;
}

namespace TF2Vulkan{ namespace Factories
{
	template<typename T>
	struct BaseObjectFactory
	{
		T& SetDebugName(std::string&& dbgName);
		T& SetDebugName(const std::string_view& dbgName);

		std::string m_DebugName;
	};

	template<typename T>
	struct VmaObjectFactory : BaseObjectFactory<T>
	{
		T& SetAllocCreateInfo(const vma::AllocationCreateInfo& allocInfo);
		T& SetMemoryType(vma::MemoryType type);
		T& SetAllowMapping(bool allow = true);

		vma::AllocationCreateInfo m_AllocInfo;
	};

	struct BufferFactory final : VmaObjectFactory<BufferFactory>
	{
		BufferFactory() = default;

		BufferFactory& SetUsage(const vk::BufferUsageFlags& usage);
		BufferFactory& SetSize(size_t size);
		BufferFactory& SetInitialData(const void* initialData, size_t initialDataSize, size_t writeOffset = 0);
		BufferFactory& SetMemoryRequiredFlags(const vk::MemoryPropertyFlags& flags);
		BufferFactory& SetDebugName(std::string&& dbgName);

		vma::AllocatedBuffer Create() const;

		vk::BufferCreateInfo m_CreateInfo;

		const void* m_InitialData = nullptr;
		size_t m_InitialDataSize = 0;
		size_t m_InitialDataWriteOffset = 0;
	};

	struct ImageFactory final : VmaObjectFactory<ImageFactory>
	{
		ImageFactory();

		ImageFactory& SetCreateInfo(const vk::ImageCreateInfo& createInfo);
		ImageFactory& AddUsageFlags(const vk::ImageUsageFlags& usage);
		ImageFactory& SetUsage(const vk::ImageUsageFlags& usage);
		ImageFactory& SetAllowMapping(bool allow = true);
		ImageFactory& SetDefaultLayout(vk::ImageLayout layout);
		ImageFactory& SetFormat(vk::Format format);
		ImageFactory& SetExtent(uint32_t width, uint32_t height, uint32_t depth = 1);
		ImageFactory& SetExtent(const vk::Extent2D& extent);

		vma::AllocatedImage Create() const;

		vk::ImageLayout m_DefaultLayout = vk::ImageLayout::eUndefined;
		vk::ImageCreateInfo m_CreateInfo;
	};

	/// <summary>
	/// A hopefully simpler version of barriers so I don't keep forgetting how
	/// they work. See https://gpuopen.com/vulkan-barriers-explained/
	/// </summary>
	struct ImageMemoryBarrierFactory final
	{
		ImageMemoryBarrierFactory();

		/// <summary>
		/// The last stage that *must* execute before the layout transition happens.
		/// This should be as high up the pipe as possible, but it's always safer
		/// to move it towards the bottom.
		/// </summary>
		ImageMemoryBarrierFactory& SetProducerStage(vk::PipelineStageFlags stage, bool read = false, bool write = true);

		/// <summary>
		/// The stage the layout transition *must* be finished by. The transition
		/// *may* happen earlier than this stage (but *not* before the producer
		/// stage). This should be as low down the pipe as possible, but it's
		/// always safer to move it towards the top.
		/// </summary>
		ImageMemoryBarrierFactory& SetConsumerStage(vk::PipelineStageFlags stage, bool read = true, bool write = false);

		ImageMemoryBarrierFactory& SetOldLayout(vk::ImageLayout oldLayout);
		ImageMemoryBarrierFactory& SetNewLayout(vk::ImageLayout newLayout);

		ImageMemoryBarrierFactory& SetAspects(vk::ImageAspectFlags aspects);
		ImageMemoryBarrierFactory& SetAspectsFromFormat(vk::Format imgFormat);
		ImageMemoryBarrierFactory& SetFullSubresourceRange(uint32_t arrayLayerCount, uint32_t mipLevelCount);
		ImageMemoryBarrierFactory& SetFullSubresourceRange(const vk::ImageCreateInfo& ci);

		ImageMemoryBarrierFactory& SetImage(vk::Image image);
		ImageMemoryBarrierFactory& SetImage(const IVulkanTexture& image,
			bool setAspects = true, bool setFullSubresourceRange = false);

		ImageMemoryBarrierFactory& SetMipLevels(uint32_t baseMipLevel, uint32_t mipLevelCount = 1);
		ImageMemoryBarrierFactory& SetArrayLayers(uint32_t baseLayer, uint32_t layerCount = 1);

		ImageMemoryBarrierFactory& Submit(IVulkanCommandBuffer& cmdBuf);
		const ImageMemoryBarrierFactory& Submit(IVulkanCommandBuffer& cmdBuf) const;

		vk::PipelineStageFlags m_SrcStage;
		vk::PipelineStageFlags m_DstStage;
		vk::ImageMemoryBarrier m_Barrier;
	};

	struct DescriptorSetLayoutFactory final
	{
		DescriptorSetLayoutFactory() = default;

		DescriptorSetLayoutFactory& AddBinding(uint32_t binding, vk::DescriptorType type,
			vk::ShaderStageFlagBits stageFlags, uint32_t arraySize = 1);

		vk::UniqueDescriptorSetLayout Create() const;

		DescriptorSetLayoutFactory& UpdateBindingsPointers();

		std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;
		vk::DescriptorSetLayoutCreateInfo m_CreateInfo;
	};
} }
