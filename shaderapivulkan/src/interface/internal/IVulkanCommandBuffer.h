#pragma once

#include "TF2Vulkan/PixScope.h"

#include <Color.h>

#include <optional>

namespace TF2Vulkan
{
	class IVulkanQueue;

	class IVulkanCommandBuffer
	{
	public:
		virtual ~IVulkanCommandBuffer() = default;

		virtual void AddResource(vk::UniqueBuffer&& buffer) = 0;
		virtual void AddResource(vma::AllocatedBuffer&& buffer) = 0;
		virtual void AddResource(vk::UniqueDescriptorSet&& descriptor) = 0;

		template<typename TContainer>
		auto AddResource(TContainer&& container) -> decltype(std::begin(container), std::end(container), AddResource(std::move(*std::begin(container))))
		{
			for (auto&& val : container)
				AddResource(std::move(val));
		}

		void InsertDebugLabel(const Color& color, const char* text);
		void InsertDebugLabel(const char* text);
		template<typename... TArgs> void InsertDebugLabel(const Color& color, const char* fmt, const TArgs&... args);
		template<typename... TArgs> void InsertDebugLabel(const char* fmt, const TArgs&... args);
		PixScope DebugRegionBegin(const Color& color, const char* text);
		PixScope DebugRegionBegin(const char* text);
		template<typename... TArgs> PixScope DebugRegionBegin(const Color& color, const char* fmt, const TArgs&... args);
		template<typename... TArgs> PixScope DebugRegionBegin(const char* fmt, const TArgs&... args);

		struct ActiveRenderPass final
		{
			vk::RenderPassBeginInfo m_BeginInfo;
			vk::SubpassContents m_Contents;
		};
		const ActiveRenderPass* GetActiveRenderPass() const;
		bool IsRenderPassActive(const vk::RenderPassBeginInfo& beginInfo, const vk::SubpassContents& contents) const;

		virtual IVulkanQueue& GetQueue() = 0;

		bool IsActive() const;
		void Submit(vk::SubmitInfo submitInfo = {}, const vk::Fence& fence = nullptr);

		void CopyBufferToImage(const vk::Buffer& buffer, const vk::Image& image, const vk::Extent2D& size, uint32_t sliceOffset);

#pragma region VkCommandBuffer Functionality

		void insertDebugUtilsLabelEXT(const vk::DebugUtilsLabelEXT& labelInfo);
		void beginDebugUtilsLabelEXT(const vk::DebugUtilsLabelEXT& labelInfo);
		void endDebugUtilsLabelEXT();

		void begin(const vk::CommandBufferBeginInfo& beginInfo);
		void beginRenderPass(const vk::RenderPassBeginInfo& renderPassBegin, const vk::SubpassContents& contents);
		void bindDescriptorSets(const vk::PipelineBindPoint& pipelineBindPoint, const vk::PipelineLayout& layout,
			uint32_t firstSet, const vk::ArrayProxy<const vk::DescriptorSet>& descriptorSets,
			const vk::ArrayProxy<const uint32_t> dynamicOffsets);
		void bindPipeline(const vk::PipelineBindPoint& pipelineBindPoint, const vk::Pipeline& pipeline);
		void bindIndexBuffer(const vk::Buffer& buffer, const vk::DeviceSize& offset, const vk::IndexType& indexType);
		void bindVertexBuffers(uint32_t firstBinding, const vk::ArrayProxy<const vk::Buffer>& buffers,
			const vk::ArrayProxy<const vk::DeviceSize>& offsets);
		void copyBufferToImage(const vk::Buffer& buf, const vk::Image& img, const vk::ImageLayout& dstImageLayout,
			const vk::ArrayProxy<const vk::BufferImageCopy>& regions);
		void clearAttachments(uint32_t attachmentCount, const vk::ClearAttachment* pAttachments,
			uint32_t rectCount, const vk::ClearRect* pRects);
		void clearAttachments(const vk::ArrayProxy<const vk::ClearAttachment>& attachments,
			const vk::ArrayProxy<const vk::ClearRect>& rects);
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
			int32_t vertexOffset = 0, uint32_t firstInstance = 0);
		void end();
		void endRenderPass();
		void pipelineBarrier(const vk::PipelineStageFlags& srcStageMask, const vk::PipelineStageFlags& dstStageMask,
			const vk::DependencyFlags& dependencyFlags, const vk::ArrayProxy<const vk::MemoryBarrier>& memoryBarriers,
			const vk::ArrayProxy<const vk::BufferMemoryBarrier>& bufferMemoryBarriers,
			const vk::ArrayProxy<const vk::ImageMemoryBarrier>& imageMemoryBarriers);

#pragma endregion VkCommandBuffer Functionality

	protected:
		virtual const vk::CommandBuffer& GetCmdBuffer() const = 0;
		virtual void ReleaseAttachedResources() = 0;

	private:
		bool m_IsActive = false; // Is inside begin()..end()
		std::optional<ActiveRenderPass> m_ActiveRenderPass;
		int m_DebugScopeCount = 0;
	};

	template<typename ...TArgs>
	inline void IVulkanCommandBuffer::InsertDebugLabel(const Color& color, const char* fmt, const TArgs& ...args)
	{
		char buf[512];
		sprintf_s(buf, fmt, args...);
		return InsertDebugLabel(color, buf);
	}

	template<typename ...TArgs>
	inline void IVulkanCommandBuffer::InsertDebugLabel(const char* fmt, const TArgs& ...args)
	{
		char buf[512];
		sprintf_s(buf, fmt, args...);
		return InsertDebugLabel(buf);
	}

	template<typename... TArgs>
	inline PixScope IVulkanCommandBuffer::DebugRegionBegin(const Color& color, const char* fmt, const TArgs&... args)
	{
		char buf[512];
		sprintf_s(buf, fmt, args...);
		return DebugRegionBegin(color, buf);
	}

	template<typename ...TArgs>
	inline PixScope IVulkanCommandBuffer::DebugRegionBegin(const char* fmt, const TArgs& ...args)
	{
		char buf[512];
		sprintf_s(buf, fmt, args...);
		return DebugRegionBegin(buf);
	}
}
