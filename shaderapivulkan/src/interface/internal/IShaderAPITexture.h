#pragma once

#include <materialsystem/itexture.h>
#include <shaderapi/ishaderdynamic.h>

namespace TF2Vulkan
{
	// FIXME: Is it safe to assume nobody will try to cast this to ITextureInternal or something?
	class IVulkanTexture
	{
	protected:
		virtual ~IVulkanTexture() = default;

	public:
		virtual std::string_view GetDebugName() const = 0;
		virtual const vk::Image& GetImage() const = 0;
		virtual const vk::ImageCreateInfo& GetImageCreateInfo() const = 0;

		virtual const vk::ImageView& FindOrCreateView(const vk::ImageViewCreateInfo& createInfo) = 0;

		virtual const vk::ImageView& FindOrCreateView();

		template<typename T1, typename T2, typename = std::enable_if_t<std::is_integral_v<T1> && std::is_integral_v<T2>>>
		void GetSize(T1& width, T2& height) const
		{
			const auto& ci = GetImageCreateInfo();
			Util::SafeConvert(ci.extent.width, width);
			Util::SafeConvert(ci.extent.height, height);
		}
	};

	class IShaderAPITexture : public IVulkanTexture, public ITexture
	{
	public:
		virtual ShaderAPITextureHandle_t GetHandle() const = 0;

		// ITexture implementation
		const char* GetName() const override final { NOT_IMPLEMENTED_FUNC(); }
		int GetMappingWidth() const override final { NOT_IMPLEMENTED_FUNC(); }
		int GetMappingHeight() const override final { NOT_IMPLEMENTED_FUNC(); }
		int GetMappingDepth() const override final { NOT_IMPLEMENTED_FUNC(); }
		int GetActualWidth() const override final;
		int GetActualHeight() const override final;
		int GetActualDepth() const override final;
		int GetNumAnimationFrames() const override final { NOT_IMPLEMENTED_FUNC(); }
		bool IsTranslucent() const override final { NOT_IMPLEMENTED_FUNC(); }
		bool IsMipmapped() const override final;

		void GetLowResColorSample(float s, float t, float* color) const override final
		{
			NOT_IMPLEMENTED_FUNC();
		}

		void* GetResourceData(uint32 eDataType, size_t* pNumBytes) const override final
		{
			NOT_IMPLEMENTED_FUNC();
		}

		void IncrementReferenceCount() override final { NOT_IMPLEMENTED_FUNC(); }
		void DecrementReferenceCount() override final { NOT_IMPLEMENTED_FUNC(); }

		void SetTextureRegenerator(ITextureRegenerator* pTextureRegen) override final { NOT_IMPLEMENTED_FUNC(); }

		void Download(Rect_t* pRect, int additionalCreationFlags) override final { NOT_IMPLEMENTED_FUNC(); }

		int GetApproximateVidMemBytes() const override final { NOT_IMPLEMENTED_FUNC(); }

		bool IsError() const override final { NOT_IMPLEMENTED_FUNC(); }

		bool IsVolumeTexture() const override final { NOT_IMPLEMENTED_FUNC(); }

		ImageFormat GetImageFormat() const override final;
		NormalDecodeMode_t GetNormalDecodeMode() const override final { NOT_IMPLEMENTED_FUNC(); }

		bool IsRenderTarget() const override final { NOT_IMPLEMENTED_FUNC(); }
		bool IsCubeMap() const override final { NOT_IMPLEMENTED_FUNC(); }
		bool IsNormalMap() const override final { NOT_IMPLEMENTED_FUNC(); }
		bool IsProcedural() const override final { NOT_IMPLEMENTED_FUNC(); }

		void DeleteIfUnreferenced() override final { NOT_IMPLEMENTED_FUNC(); }

		void SwapContents(ITexture* pOther) override final { NOT_IMPLEMENTED_FUNC(); }

		unsigned int GetFlags() const override final { NOT_IMPLEMENTED_FUNC(); }

		void ForceLODOverride(int iNumLodsOverrideUpOrDown) override final { NOT_IMPLEMENTED_FUNC(); }

		bool SaveToFile(const char* fileName) override final { NOT_IMPLEMENTED_FUNC(); }

		void CopyToStagingTexture(ITexture* pDstTex) override final { NOT_IMPLEMENTED_FUNC(); }

		void SetErrorTexture(bool isErrorTexture) override final { NOT_IMPLEMENTED_FUNC(); }
	};
}
