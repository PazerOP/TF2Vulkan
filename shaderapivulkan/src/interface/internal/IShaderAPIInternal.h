#pragma once

#include "TF2Vulkan/IShaderAPI_DSA.h"
#include <TF2Vulkan/Util/ImageManip.h>

enum RenderParamFloat_t;
enum RenderParamInt_t;
enum RenderParamVector_t;

namespace TF2Vulkan
{
	struct TextureData;

	class IShaderAPIInternal : public IShaderAPI_DSA
	{
	public:
		virtual void CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t texHandle, int renderTargetID,
			const Rect_t* srcRect, const Rect_t* dstRect) = 0;
		void CopyRenderTargetToTextureEx(ShaderAPITextureHandle_t texHandle, int renderTargetID,
			Rect_t* srcRect, Rect_t* dstRect) override final
		{
			return CopyRenderTargetToTextureEx(texHandle, renderTargetID,
				const_cast<const Rect_t*>(srcRect), const_cast<const Rect_t*>(dstRect));
		}

		virtual void CopyTextureToRenderTargetEx(int renderTargetID, ShaderAPITextureHandle_t texHandle,
			const Rect_t* srcRect, const Rect_t* dstRect) = 0;
		void CopyTextureToRenderTargetEx(int renderTargetID, ShaderAPITextureHandle_t texHandle,
			Rect_t* srcRect, Rect_t* dstRect) override final
		{
			return CopyTextureToRenderTargetEx(renderTargetID, texHandle,
				const_cast<const Rect_t*>(srcRect), const_cast<const Rect_t*>(dstRect));
		}

		virtual void SetFloatRenderingParameter(RenderParamFloat_t param, float value) = 0;
		void SetFloatRenderingParameter(int param, float value) override final
		{
			return SetFloatRenderingParameter(RenderParamFloat_t(param), value);
		}

		virtual void SetIntRenderingParameter(RenderParamInt_t param, int value) = 0;
		void SetIntRenderingParameter(int param, int value) override final
		{
			return SetIntRenderingParameter(RenderParamInt_t(param), value);
		}

		virtual void SetVectorRenderingParameter(RenderParamVector_t param, const Vector& value) = 0;
		void SetVectorRenderingParameter(int param, const Vector& value) override final
		{
			return SetVectorRenderingParameter(RenderParamVector_t(param), value);
		}

		virtual float GetFloatRenderingParameter(RenderParamFloat_t param) const = 0;
		float GetFloatRenderingParameter(int param) const override final
		{
			return GetFloatRenderingParameter(RenderParamFloat_t(param));
		}

		virtual int GetIntRenderingParameter(RenderParamInt_t param) const = 0;
		int GetIntRenderingParameter(int param) const override final
		{
			return GetIntRenderingParameter(RenderParamInt_t(param));
		}

		virtual Vector GetVectorRenderingParameter(RenderParamVector_t param) const = 0;
		Vector GetVectorRenderingParameter(int param) const override final
		{
			return GetVectorRenderingParameter(RenderParamVector_t(param));
		}

		virtual void CopyRenderTargetToScratchTexture(ShaderAPITextureHandle_t srcRT,
			ShaderAPITextureHandle_t dstTex, const Rect_t* srcRect, const Rect_t* dstRect) = 0;
		void CopyRenderTargetToScratchTexture(ShaderAPITextureHandle_t srcRT,
			ShaderAPITextureHandle_t dstTex, Rect_t* srcRect, Rect_t* dstRect) override final
		{
			return CopyRenderTargetToScratchTexture(srcRT, dstTex,
				const_cast<const Rect_t*>(srcRect), const_cast<const Rect_t*>(dstRect));
		}

		virtual bool IsInFrame() const = 0;
		virtual bool IsInPass() const = 0;
		virtual bool GetTextureSize(ShaderAPITextureHandle_t texID, uint_fast32_t& width, uint_fast32_t& height) const = 0;
	};

	extern IShaderAPIInternal& g_ShaderAPIInternal;
}
