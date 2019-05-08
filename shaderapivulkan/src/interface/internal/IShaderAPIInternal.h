#pragma once

#include "TF2Vulkan/ShaderDevice.h"
#include "TF2Vulkan/IShaderAPI_DSA.h"
#include "interface/internal/IShaderAPITexture.h"
#include <TF2Vulkan/Util/ImageManip.h>
#include <TF2Vulkan/Util/ScopeFunc.h>

#include <Color.h>

#include <stack>

enum RenderParamFloat_t;
enum RenderParamInt_t;
enum RenderParamVector_t;

namespace TF2Vulkan
{
	struct TextureData;
	union VertexFormat;
	class VulkanMesh;
	class VulkanIndexBuffer;
	class VulkanVertexBuffer;

	struct ActiveMeshData
	{
		VulkanMesh* m_Mesh;
		int m_FirstIndex;
		int m_IndexCount;
	};

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

		virtual const IShaderAPITexture* TryGetTexture(ShaderAPITextureHandle_t texID) const = 0;
		virtual const IShaderAPITexture& TryGetTexture(ShaderAPITextureHandle_t texID, StandardTextureId_t fallbackID) const = 0;
		IShaderAPITexture* TryGetTexture(ShaderAPITextureHandle_t texID)
		{
			return const_cast<IShaderAPITexture*>(std::as_const(*this).TryGetTexture(texID));
		}
		IShaderAPITexture& TryGetTexture(ShaderAPITextureHandle_t texID, StandardTextureId_t fallbackID)
		{
			return const_cast<IShaderAPITexture&>(std::as_const(*this).TryGetTexture(texID, fallbackID));
		}
		const IShaderAPITexture& GetTexture(ShaderAPITextureHandle_t texID) const
		{
			auto found = TryGetTexture(texID);
			if (!found)
				throw VulkanException("TryGetTexture returned nullptr", EXCEPTION_DATA());

			return *found;
		}
		IShaderAPITexture& GetTexture(ShaderAPITextureHandle_t texID)
		{
			return const_cast<IShaderAPITexture&>(std::as_const(*this).GetTexture(texID));
		}

		using IShaderAPI_DSA::CreateTexture;
		virtual IShaderAPITexture& CreateTexture(std::string&& dbgName, const vk::ImageCreateInfo& imgCI) = 0;

		void GetBackBufferDimensions(uint32_t& width, uint32_t& height) const
		{
			return g_ShaderDevice.GetBackBufferDimensions(width, height);
		}
		void GetBackBufferDimensions(int& width, int& height) const override final
		{
			LOG_FUNC();
			return g_ShaderDevice.GetBackBufferDimensions(width, height);
		}

		virtual void SetPIXMarker(const Color& color, const char* name) = 0;
		[[deprecated]] void SetPIXMarker(unsigned long color, const char* name) override final
		{
			Color col;
			col.SetRawColor(color);
			return SetPIXMarker(col, name);
		}

		virtual const ActiveMeshData& GetActiveMesh() = 0;
		virtual void PushActiveMesh(const ActiveMeshData& mesh) = 0;
		virtual void PopActiveMesh() = 0;
	};

	extern IShaderAPIInternal& g_ShaderAPIInternal;

	namespace detail
	{
		struct ActiveMeshPusher final
		{
			constexpr ActiveMeshPusher(ActiveMeshData&& meshData) : m_MeshData(meshData) {}
			ActiveMeshData m_MeshData;
			void operator()() const
			{
				g_ShaderAPIInternal.PushActiveMesh(m_MeshData);
			}
		};
		struct ActiveMeshPopper final
		{
			void operator()() const
			{
				g_ShaderAPIInternal.PopActiveMesh();
			}
		};
	}

	using ActiveMeshScope = Util::ScopeFunc<detail::ActiveMeshPusher, detail::ActiveMeshPopper>;
}
