#pragma once

#include "TF2Vulkan/PixScope.h"
#include "TF2Vulkan/ResourceBlob.h"

#include <Color.h>

#include <optional>

namespace TF2Vulkan
{
	class IVulkanQueue;

	static constexpr Color PIX_COLOR_MISC(255, 93, 79);
	static constexpr Color PIX_COLOR_DRAW(128, 255, 128);
	static constexpr Color PIX_COLOR_CLEAR(0, 0, 0);
	static constexpr Color PIX_COLOR_READWRITE(240, 228, 66);
	static constexpr Color PIX_COLOR_READ(230, 159, 0);
	static constexpr Color PIX_COLOR_WRITE(86, 180, 233);

	class IVulkanCommandBuffer : public ResourceBlob
	{
	public:
		virtual ~IVulkanCommandBuffer() = default;

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
		bool TryEndRenderPass();

		virtual IVulkanQueue& GetQueue() = 0;

		bool IsActive() const;
		void Submit(vk::SubmitInfo submitInfo = {}, const vk::Fence& fence = nullptr);

		void CopyBufferToImage(const vk::Buffer& buffer, const vk::Image& image, const vk::Extent2D& size, uint32_t sliceOffset);

#pragma region VkCommandBuffer Functionality

		void insertDebugUtilsLabelEXT(const vk::DebugUtilsLabelEXT& labelInfo);
		void beginDebugUtilsLabelEXT(const vk::DebugUtilsLabelEXT& labelInfo);
		void endDebugUtilsLabelEXT();

		void begin(const vk::CommandBufferBeginInfo& beginInfo);
		void end();
		void reset(vk::CommandBufferResetFlags flags = {});
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
		void endRenderPass();
		void pipelineBarrier(const vk::PipelineStageFlags& srcStageMask, const vk::PipelineStageFlags& dstStageMask,
			const vk::DependencyFlags& dependencyFlags, const vk::ArrayProxy<const vk::MemoryBarrier>& memoryBarriers,
			const vk::ArrayProxy<const vk::BufferMemoryBarrier>& bufferMemoryBarriers,
			const vk::ArrayProxy<const vk::ImageMemoryBarrier>& imageMemoryBarriers);
		void blitImage(const vk::Image& srcImg, const vk::ImageLayout& srcImgLayout,
			const vk::Image& dstImg, const vk::ImageLayout& dstImgLayout,
			const vk::ArrayProxy<const vk::ImageBlit>& regions, const vk::Filter& filter);
		void copyImage(const vk::Image& srcImg, const vk::ImageLayout& srcImgLayout,
			const vk::Image& dstImg, const vk::ImageLayout& dstImgLayout,
			const vk::ArrayProxy<const vk::ImageCopy>& regions);

#pragma endregion VkCommandBuffer Functionality

	protected:
		virtual const vk::CommandBuffer& GetCmdBuffer() const = 0;

	private:
		void ResetActiveState();

		bool m_IsActive = false; // Is inside begin()..end()
		std::optional<ActiveRenderPass> m_ActiveRenderPass;
		vk::Pipeline m_ActivePipeline;
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
