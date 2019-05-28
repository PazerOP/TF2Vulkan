#pragma once

#include "interface/internal/IShaderAPIInternal.h"
#include "TF2Vulkan/SamplerSettings.h"
#include "TF2Vulkan/TextureSubrect.h"
#include "TF2Vulkan/VulkanFactories.h"

#include <array>
#include <atomic>
#include <unordered_map>

namespace TF2Vulkan
{
	class IShaderAPI_TextureManager : public IShaderAPIInternal
	{
	public:
		const IShaderAPITexture* TryGetTexture(ShaderAPITextureHandle_t texID) const;
		const IShaderAPITexture& TryGetTexture(ShaderAPITextureHandle_t texID, StandardTextureId_t fallbackID) const;
		IShaderAPITexture* TryGetTexture(ShaderAPITextureHandle_t texID);

		IShaderAPITexture& TryGetTexture(ShaderAPITextureHandle_t texID, StandardTextureId_t fallbackID);
		const IShaderAPITexture& GetTexture(ShaderAPITextureHandle_t texID) const;
		IShaderAPITexture& GetTexture(ShaderAPITextureHandle_t texID);
		ShaderAPITextureHandle_t GetStdTextureHandle(StandardTextureId_t stdTex) const;

		void TexImageFromVTF(ShaderAPITextureHandle_t texHandle, IVTFTexture* vtf, int ivtfFrame);
		bool UpdateTexture(ShaderAPITextureHandle_t texHandle, const TextureData* data,
			size_t count);
		void TexMinFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode);
		void TexMagFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode);
		void TexWrap(ShaderAPITextureHandle_t tex, ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode);
		void TexSetPriority(ShaderAPITextureHandle_t tex, int priority);

		using IShaderAPI::CreateTexture;
		IShaderAPITexture& CreateTexture(std::string&& dbgName, const vk::ImageCreateInfo& imgCI);
		IShaderAPITexture& CreateTexture(std::string&& dbgName, const vk::ImageCreateInfo& imgCI,
			const vma::AllocationCreateInfo& allocCI);
		IShaderAPITexture& CreateTexture(Factories::ImageFactory&& factory);
		IShaderAPITexture& CreateTexture(std::string&& dbgName, Factories::ImageFactory&& factory);
		ShaderAPITextureHandle_t CreateTexture(int width, int height, int depth, ImageFormat dstImgFormat,
			int mipLevelCount, int copyCount, CreateTextureFlags_t flags, const char* dbgName, const char* texGroupName) override final;
		ShaderAPITextureHandle_t CreateDepthTexture(ImageFormat rtFormat, int width,
			int height, const char* dbgName, bool texture) override final;
		void CreateTextures(ShaderAPITextureHandle_t* handles, int count, int width, int height, int depth,
			ImageFormat dstImgFormat, int mipLevelCount, int copyCount, CreateTextureFlags_t flags, const char* dbgName,
			const char* texGroupName) override final;
		void DeleteTexture(ShaderAPITextureHandle_t tex) override final;
		bool IsTexture(ShaderAPITextureHandle_t tex) override final;

		bool IsTextureResident(ShaderAPITextureHandle_t tex) override final { NOT_IMPLEMENTED_FUNC(); }
		void SetStandardTextureHandle(StandardTextureId_t id, ShaderAPITextureHandle_t tex) override final;
		void GetStandardTextureDimensions(int* width, int* height, StandardTextureId_t id) override final;

		void LockRect(void** outBits, int* outPitch, ShaderAPITextureHandle_t tex, int mipLevel,
			int x, int y, int w, int h, bool write, bool read) override final;
		void UnlockRect(ShaderAPITextureHandle_t tex, int mipLevel) override final;

		bool TexLock(ShaderAPITextureHandle_t tex, int level, int cubeFaceID, int xOffset, int yOffset,
			int width, int height, CPixelWriter& writer);
		void TexUnlock(ShaderAPITextureHandle_t tex);

		bool TexLock(ShaderAPITextureHandle_t tex, TextureData& data, bool read, bool write);

	private:
		std::atomic<ShaderAPITextureHandle_t> m_NextTextureHandle = 1;

		struct LockedTextureRect final
		{
			std::unique_ptr<std::byte[]> m_Data;
			size_t m_DataLength = 0;
			TextureSubrect m_Subrect;
			bool m_IsDirectMapped = false;

			operator bool() const
			{
				assert(!(m_IsDirectMapped && m_Data));
				assert(!m_DataLength == !m_Data);
				return m_IsDirectMapped || m_Data;
			}
		};

		struct ShaderTexture final : public IShaderAPITexture
		{
			ShaderTexture(std::string&& debugName, ShaderAPITextureHandle_t handle,
				const Factories::ImageFactory& ci, vma::AllocatedImage&& img);
			//~ShaderTexture();

			SamplerSettings m_SamplerSettings;

			std::string_view GetDebugName() const override { return m_DebugName; }
			vk::Image GetImage() const override { return m_Image.GetImage(); }
			const vk::ImageCreateInfo& GetImageCreateInfo() const override { return m_Factory.m_CreateInfo; }
			vk::ImageView FindOrCreateView(const vk::ImageViewCreateInfo& createInfo) override;
			ShaderAPITextureHandle_t GetHandle() const override { return m_Handle; }
			vk::ImageLayout GetDefaultLayout() const override { return m_Factory.m_DefaultLayout; }

			std::string m_DebugName;
			vma::AllocatedImage m_Image;
			std::unordered_map<vk::ImageViewCreateInfo, vk::UniqueImageView> m_ImageViews;

			static constexpr auto MAX_MIPLEVELS = 14;
			std::array<LockedTextureRect, MAX_MIPLEVELS> m_LockedRects;

		private:
			Factories::ImageFactory m_Factory;
			ShaderAPITextureHandle_t m_Handle;
		};
		std::unordered_map<ShaderAPITextureHandle_t, ShaderTexture> m_Textures;

		std::array<ShaderAPITextureHandle_t, TEXTURE_MAX_STD_TEXTURES> m_StdTextures;

		// IShaderAPI global state based API
		ShaderAPITextureHandle_t m_ModifyTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
		void ModifyTexture(ShaderAPITextureHandle_t tex) override final;
		void TexImage2D(int level, int cubeFaceID, ImageFormat dstFormat, int zOffset,
			int width, int height, ImageFormat srcFormat, bool srcIsTiled,
			void* imgData) override final;
		void TexSubImage2D(int level, int cubeFaceID, int xOffset, int yOffset, int zOffset,
			int width, int height, ImageFormat srcFormat, int srcStride, bool srcIsTiled,
			void* imgData) override final;
		void TexImageFromVTF(IVTFTexture* vtf, int frameIndex) override final;
		void TexMinFilter(ShaderTexFilterMode_t mode) override final;
		void TexMagFilter(ShaderTexFilterMode_t mode) override final;
		void TexWrap(ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode) override final;
		void TexLodClamp(int something) override final;
		void TexLodBias(float bias) override final;
		void TexSetPriority(int priority) override final;

		bool TexLock(int level, int cubeFaceID, int xOffset, int yOffset,
			int width, int height, CPixelWriter& writer) override final;
		void TexUnlock() override final;
	};

	extern IShaderAPI_TextureManager& g_TextureManager;
}
