#pragma once

#include "interface/internal/IShaderAPIInternal.h"
#include "TF2Vulkan/SamplerSettings.h"

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

		using IShaderAPI::CreateTexture;
		IShaderAPITexture& CreateTexture(std::string&& dbgName, const vk::ImageCreateInfo& imgCI);
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

	private:
		std::atomic<ShaderAPITextureHandle_t> m_NextTextureHandle = 1;

		struct ShaderTexture : IShaderAPITexture
		{
			ShaderTexture(std::string&& debugName, ShaderAPITextureHandle_t handle,
				const vk::ImageCreateInfo& ci, vma::AllocatedImage&& img);

			std::string m_DebugName;
			vk::ImageCreateInfo m_CreateInfo;
			vma::AllocatedImage m_Image;
			ShaderAPITextureHandle_t m_Handle;
			std::unordered_map<vk::ImageViewCreateInfo, vk::UniqueImageView> m_ImageViews;

			SamplerSettings m_SamplerSettings;

			std::string_view GetDebugName() const override { return m_DebugName; }
			const vk::Image& GetImage() const override { return m_Image.GetImage(); }
			const vk::ImageCreateInfo& GetImageCreateInfo() const override { return m_CreateInfo; }
			const vk::ImageView& FindOrCreateView(const vk::ImageViewCreateInfo& createInfo) override;
			ShaderAPITextureHandle_t GetHandle() const override { return m_Handle; }
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
	};

	extern IShaderAPI_TextureManager& g_TextureManager;
}
