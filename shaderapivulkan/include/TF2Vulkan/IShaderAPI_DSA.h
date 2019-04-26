#pragma once

#include <TF2Vulkan/TextureData.h>

#include <shaderapi/ishaderapi.h>

namespace TF2Vulkan
{
	class IShaderAPI_DSA : public IShaderAPI
	{
	public:
		virtual bool UpdateTexture(ShaderAPITextureHandle_t texHandle,
			const TextureData* data, size_t count) = 0;

		template<size_t size> bool UpdateTexture(ShaderAPITextureHandle_t texHandle,
			const TextureData(&data)[size])
		{
			return UpdateTexture(texHandle, data, size);
		}

		virtual void TexMinFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode) = 0;
		virtual void TexMagFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode) = 0;
		virtual void TexWrap(ShaderAPITextureHandle_t texHandle,
			ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode) = 0;
		virtual void TexImageFromVTF(ShaderAPITextureHandle_t texHandle, IVTFTexture* vtf, int frameIndex) = 0;

	private:
		void ModifyTexture(ShaderAPITextureHandle_t tex) override final
		{
			LOG_FUNC();
			m_ModifyTexture = tex;
		}

		// Legacy shim
		void TexImage2D(int level, int cubeFaceID, ImageFormat dstFormat, int zOffset,
			int width, int height, ImageFormat srcFormat, bool srcIsTiled,
			void* imgData) override final
		{
			LOG_FUNC();

			if (dstFormat != srcFormat)
				NOT_IMPLEMENTED_FUNC();

			return TexSubImage2D(level, cubeFaceID,
				0, 0, zOffset,
				width, height,
				srcFormat,
				-1,
				srcIsTiled,
				imgData);
		}

		// Legacy shim
		void TexSubImage2D(int level, int cubeFaceID, int xOffset, int yOffset, int zOffset,
			int width, int height, ImageFormat srcFormat, int srcStride, bool srcIsTiled,
			void* imgData) override final
		{
			LOG_FUNC();

			assert(!srcIsTiled); // Not valid on PC

			TextureData data;
			data.m_CubeFace = CubeMapFaceIndex_t(cubeFaceID);
			data.m_Format = srcFormat;
			Util::SafeConvert(level, data.m_MipLevel);
			Util::SafeConvert(xOffset, data.m_XOffset);
			Util::SafeConvert(yOffset, data.m_YOffset);
			Util::SafeConvert(zOffset, data.m_ZOffset);
			Util::SafeConvert(width, data.m_Width);
			Util::SafeConvert(height, data.m_Height);
			Util::SafeConvert(srcStride, data.m_Stride);
			data.m_Data = imgData;

			data.Validate();

			UpdateTexture(m_ModifyTexture, &data, 1);
		}

		// Legacy shim
		void TexImageFromVTF(IVTFTexture* vtf, int frameIndex) override final
		{
			LOG_FUNC();
			return TexImageFromVTF(m_ModifyTexture, vtf, frameIndex);
		}

		// Legacy shim
		void TexMinFilter(ShaderTexFilterMode_t mode) override final
		{
			LOG_FUNC();
			return TexMinFilter(m_ModifyTexture, mode);
		}
		// Legacy shim
		void TexMagFilter(ShaderTexFilterMode_t mode) override final
		{
			LOG_FUNC();
			return TexMagFilter(m_ModifyTexture, mode);
		}
		// Legacy shim
		void TexWrap(ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode) override final
		{
			LOG_FUNC();
			return TexWrap(m_ModifyTexture, coord, wrapMode);
		}

		ShaderAPITextureHandle_t m_ModifyTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
	};
}
