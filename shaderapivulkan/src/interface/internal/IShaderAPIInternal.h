#pragma once

#include "interface/internal/IShaderAPITexture.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include <TF2Vulkan/Util/ImageManip.h>
#include <TF2Vulkan/Util/ScopeFunc.h>

#include <Color.h>
#include <shaderapi/ishaderapi.h>

#include <stack>

namespace TF2Vulkan
{
	struct TextureData;
	union VertexFormat;
	class VulkanMesh;

	struct ActiveMeshData
	{
		VulkanMesh* m_Mesh;
		int m_FirstIndex;
		int m_IndexCount;
	};

	class IShaderAPIInternal : public IShaderAPI
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
