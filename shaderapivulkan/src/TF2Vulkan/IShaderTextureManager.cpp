#include "IShaderTextureManager.h"
#include "FormatConversion.h"
#include "VulkanFactories.h"

#include <TF2Vulkan/Util/std_string.h>

#define LOG_FUNC_TEX_NAME(texHandle, texName) \
	LOG_FUNC_MSG(texName)

#define LOG_FUNC_TEX(texHandle) \
	LOG_FUNC_TEX_NAME((texHandle), (m_Textures.at(texHandle).GetDebugName()))

#undef min
#undef max

using namespace TF2Vulkan;

const IShaderAPITexture* IShaderTextureManager::TryGetTexture(ShaderAPITextureHandle_t texID) const
{
	if (auto found = m_Textures.find(texID); found != m_Textures.end())
		return &found->second;

	return nullptr;
}

const IShaderAPITexture& IShaderTextureManager::TryGetTexture(ShaderAPITextureHandle_t texID, StandardTextureId_t fallbackID) const
{
	if (auto found = TryGetTexture(texID))
		return *found;

	return GetTexture(m_StdTextures.at(fallbackID));
}

IShaderAPITexture* IShaderTextureManager::TryGetTexture(ShaderAPITextureHandle_t texID)
{
	return const_cast<IShaderAPITexture*>(std::as_const(*this).TryGetTexture(texID));
}

IShaderAPITexture& IShaderTextureManager::TryGetTexture(ShaderAPITextureHandle_t texID, StandardTextureId_t fallbackID)
{
	return const_cast<IShaderAPITexture&>(std::as_const(*this).TryGetTexture(texID, fallbackID));
}

const IShaderAPITexture& IShaderTextureManager::GetTexture(ShaderAPITextureHandle_t texID) const
{
	auto found = TryGetTexture(texID);
	if (!found)
		throw VulkanException("TryGetTexture returned nullptr", EXCEPTION_DATA());

	return *found;
}

IShaderAPITexture& IShaderTextureManager::GetTexture(ShaderAPITextureHandle_t texID)
{
	return const_cast<IShaderAPITexture&>(std::as_const(*this).GetTexture(texID));
}

ShaderAPITextureHandle_t IShaderTextureManager::GetStdTextureHandle(StandardTextureId_t stdTex) const
{
	return m_StdTextures.at(stdTex);
}

void IShaderTextureManager::TexMinFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);
	tex.m_SamplerSettings.m_MinFilter = mode;
}

void IShaderTextureManager::TexMagFilter(ShaderAPITextureHandle_t texHandle, ShaderTexFilterMode_t mode)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);
	tex.m_SamplerSettings.m_MagFilter = mode;
}

IShaderAPITexture& IShaderTextureManager::CreateTexture(std::string&& dbgName, const vk::ImageCreateInfo& imgCI)
{
	const auto handle = m_NextTextureHandle++;
	LOG_FUNC_TEX_NAME(handle, dbgName);

	dbgName = Util::string::concat("[", handle, "] ", std::move(dbgName));

	auto createdImg = Factories::ImageFactory{}
		.SetCreateInfo(imgCI)
		.SetMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
		.SetDebugName(Util::string::concat("Texture: ", dbgName))
		.Create();

	return m_Textures.emplace(handle, ShaderTexture{ std::move(dbgName), handle, imgCI, std::move(createdImg) }).first->second;
}

ShaderAPITextureHandle_t IShaderTextureManager::CreateTexture(int width, int height, int depth,
	ImageFormat dstImgFormat, int mipLevelCount, int copyCount, CreateTextureFlags_t flags,
	const char* dbgName, const char* texGroupName)
{
	LOG_FUNC();
	ENSURE(width > 0);
	ENSURE(height > 0);
	ENSURE(depth > 0);
	ENSURE(mipLevelCount > 0);

	vk::ImageCreateInfo createInfo;

	// Don't actually use 1D textures, they cause issues
	/*if (height == 1 && depth == 1)
		createInfo.imageType = vk::ImageType::e1D;
	else*/ if (depth == 1)
		createInfo.imageType = vk::ImageType::e2D;
	else
		createInfo.imageType = vk::ImageType::e3D;

	Util::SafeConvert(width, createInfo.extent.width);
	Util::SafeConvert(height, createInfo.extent.height);
	Util::SafeConvert(depth, createInfo.extent.depth);
	Util::SafeConvert(mipLevelCount, createInfo.mipLevels);

	createInfo.arrayLayers = 1; // No support for texture arrays in stock valve materialsystem
	Util::SafeConvert(mipLevelCount, createInfo.mipLevels);
	createInfo.usage = vk::ImageUsageFlagBits::eSampled;

	if (flags & TEXTURE_CREATE_CUBEMAP)
		createInfo.arrayLayers = 6;

	FormatUsage fmtUsage;
	assert(!(flags & TEXTURE_CREATE_RENDERTARGET) || !(flags & TEXTURE_CREATE_DEPTHBUFFER));

	vk::ImageLayout targetLayout = vk::ImageLayout::eUndefined;
	if (flags & TEXTURE_CREATE_RENDERTARGET)
	{
		fmtUsage = FormatUsage::RenderTarget;
		createInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
		targetLayout = vk::ImageLayout::eColorAttachmentOptimal;
	}
	else if (flags & TEXTURE_CREATE_DEPTHBUFFER)
	{
		fmtUsage = FormatUsage::DepthStencil;
		createInfo.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		targetLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}
	else
	{
		fmtUsage = FormatUsage::ImmutableTexture;
		createInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
	}

	createInfo.format = ConvertImageFormat(PromoteToHardware(dstImgFormat, fmtUsage, true));

	// Make sure it's a multiple of the block size
	{
		const auto blockSize = TF2Vulkan::GetBlockSize(createInfo.format);
		const auto wDelta = createInfo.extent.width % blockSize.width;
		const auto hDelta = createInfo.extent.height % blockSize.height;

		createInfo.extent.width += (blockSize.width - wDelta) % blockSize.width;
		createInfo.extent.height += (blockSize.height - hDelta) % blockSize.height;
	}

	auto& newTex = CreateTexture(dbgName, createInfo);

	if (targetLayout != vk::ImageLayout::eUndefined)
	{
		assert(newTex.GetImageCreateInfo().mipLevels == 1);
		TransitionImageLayout(newTex.GetImage(), newTex.GetImageCreateInfo().format,
			vk::ImageLayout::eUndefined, targetLayout,
			g_ShaderDevice.GetPrimaryCmdBuf(), 0);
	}

	return newTex.GetHandle();
}

void IShaderTextureManager::CreateTextures(ShaderAPITextureHandle_t * handles, int count,
	int width, int height, int depth, ImageFormat dstImgFormat, int mipLevelCount,
	int copyCount, CreateTextureFlags_t flags, const char* dbgName, const char* texGroupName)
{
	LOG_FUNC();

	for (int i = 0; i < count; i++)
	{
		handles[i] = CreateTexture(width, height, depth, dstImgFormat,
			mipLevelCount, copyCount, flags, dbgName, texGroupName);
	}
}

void IShaderTextureManager::TexWrap(ShaderAPITextureHandle_t texHandle, ShaderTexCoordComponent_t coord, ShaderTexWrapMode_t wrapMode)
{
	LOG_FUNC();
	auto& tex = m_Textures.at(texHandle);

	auto& ss = tex.m_SamplerSettings;
	switch (coord)
	{
	case SHADER_TEXCOORD_S:
		ss.m_WrapS = wrapMode;
		break;
	case SHADER_TEXCOORD_T:
		ss.m_WrapT = wrapMode;
		break;
	case SHADER_TEXCOORD_U:
		ss.m_WrapU = wrapMode;
		break;

	default:
		Warning(TF2VULKAN_PREFIX "Invalid wrapMode %i\n", int(wrapMode));
	}
}

ShaderAPITextureHandle_t IShaderTextureManager::CreateDepthTexture(ImageFormat rtFormat, int width, int height,
	const char* dbgName, bool texture)
{
	LOG_FUNC();

	assert(!texture); // TODO: what does (texture = true) indicate

	return CreateTexture(width, height, 1, IMAGE_FORMAT_NV_DST24, 1, 0, TEXTURE_CREATE_DEPTHBUFFER, dbgName, TEXTURE_GROUP_RENDER_TARGET);
}
void IShaderTextureManager::DeleteTexture(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC_TEX(tex);

	auto& realTex = m_Textures.at(tex);

	// Print deletion
	{
		char buf[512];
		sprintf_s(buf, TF2VULKAN_PREFIX "DELETE TEXTURE \"%.*s\"\n", PRINTF_SV(realTex.GetDebugName()));
		Plat_DebugString(buf);
	}

	// Update debug name
	{
		const auto dbgName = realTex.GetDebugName();

		char buf[128];
		sprintf_s(buf, "[DELETED] Texture: %.*s", PRINTF_SV(dbgName));
		g_ShaderDevice.SetDebugName(realTex.m_Image.GetImage(), buf);

		for (auto& iv : realTex.m_ImageViews)
		{
			sprintf_s(buf, "[DELETED] ImageView: %.*s", PRINTF_SV(dbgName));
			g_ShaderDevice.SetDebugName(iv.second, buf);
		}
	}

	m_Textures.erase(tex);
}

bool IShaderTextureManager::IsTexture(ShaderAPITextureHandle_t tex)
{
	LOG_FUNC_TEX(tex);

	bool found = m_Textures.find(tex) != m_Textures.end();
	assert(found);
	return found;
}

void IShaderTextureManager::TexImageFromVTF(ShaderAPITextureHandle_t texHandle, IVTFTexture* vtf, int frameIndex)
{
	LOG_FUNC_TEX(texHandle);

	auto& tex = m_Textures.at(texHandle);

	const auto mipCount = std::min(Util::SafeConvert<uint32_t>(vtf->MipCount()), tex.m_CreateInfo.mipLevels);
	ENSURE(mipCount > 0);

	auto faceCount = vtf->FaceCount();
	ENSURE(faceCount > 0);
	if (faceCount == CUBEMAP_FACE_COUNT)
		faceCount = 6; // Drop the fallback spheremap on vulkan

	const auto arraySize = mipCount * faceCount;
	auto * texDatas = (TextureData*)stackalloc(arraySize * sizeof(TextureData));

	const auto format = vtf->Format();
	const auto blockSize = GetBlockSize(format);

	for (uint32_t mip = 0; mip < mipCount; mip++)
	{
		int width, height, depth;
		vtf->ComputeMipLevelDimensions(mip, &width, &height, &depth);

		const int mipSize = vtf->ComputeMipSize(mip);
		const int stride = vtf->RowSizeInBytes(mip);

		for (int face = 0; face < faceCount; face++)
		{
			TextureData& texData = texDatas[mip * faceCount + face];
			texData = {};
			texData.m_Format = format;

			Util::SafeConvert(width, texData.m_Width);
			Util::SafeConvert(height, texData.m_Height);
			Util::SafeConvert(depth, texData.m_Depth);
			texData.m_Data = vtf->ImageData(frameIndex, face, mip);
			Util::SafeConvert(mipSize, texData.m_DataLength);
			Util::SafeConvert(stride, texData.m_Stride);
			Util::SafeConvert(mipSize, texData.m_SliceStride);

			Util::SafeConvert(mip, texData.m_MipLevel);
			texData.m_CubeFace = CubeMapFaceIndex_t(face);

			// Clamp to min size
			texData.m_Width = std::max(texData.m_Width, blockSize.width);
			texData.m_Height = std::max(texData.m_Height, blockSize.height);

			texData.Validate();
		}
	}

	UpdateTexture(texHandle, texDatas, arraySize);
}

bool IShaderTextureManager::UpdateTexture(ShaderAPITextureHandle_t texHandle, const TextureData* data, size_t count)
{
	LOG_FUNC_TEX(texHandle);

	if (!g_ShaderDevice.IsReady())
	{
		Warning(TF2VULKAN_PREFIX "Shader device not ready\n");
		return false;
	}

	auto& tex = m_Textures.at(texHandle);

	auto& device = g_ShaderDevice.GetVulkanDevice();
	auto& alloc = g_ShaderDevice.GetVulkanAllocator();
	auto& queue = g_ShaderDevice.GetGraphicsQueue();

	std::vector<vk::BufferImageCopy> copyRegions;

	// Prepare the staging buffer
	vma::AllocatedBuffer stagingBuf;
	{
		size_t totalSize = 0;
		for (size_t i = 0; i < count; i++)
			totalSize += data[i].m_DataLength;

		// Allocate staging buffer
		stagingBuf = Factories::BufferFactory{}
			.SetSize(totalSize)
			.SetUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.SetMemoryRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			.SetDebugName(Util::string::concat(tex.m_DebugName, ": UpdateTexture() staging buffer"))
			.Create();

		// Copy the data into the staging buffer
		auto mapped = stagingBuf.GetAllocation().map();
		size_t currentOffset = 0;
		for (size_t i = 0; i < count; i++)
		{
			const TextureData& slice = data[i];

			mapped.Write(slice.m_Data, slice.m_DataLength, currentOffset);

			// Record this copy region
			{
				vk::BufferImageCopy& region = copyRegions.emplace_back();
				region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				region.imageSubresource.baseArrayLayer = slice.m_CubeFace;
				region.imageSubresource.layerCount = 1;
				region.imageSubresource.mipLevel = slice.m_MipLevel;

				region.bufferOffset = currentOffset;
				if (slice.m_Stride > 0)
					region.bufferRowLength = slice.m_Stride / (slice.m_Stride / slice.m_Width); // bufferRowLength is in texels

				if (slice.m_SliceStride > 0)
				{
					// bufferImageHeight is in texels
					if (slice.m_Stride > 0)
						region.bufferImageHeight = slice.m_SliceStride / slice.m_Stride;
					else
						region.bufferImageHeight = slice.m_Height;
				}

				Util::SafeConvert(slice.m_XOffset, region.imageOffset.x);
				Util::SafeConvert(slice.m_YOffset, region.imageOffset.y);
				Util::SafeConvert(slice.m_ZOffset, region.imageOffset.z);

				Util::SafeConvert(slice.m_Width, region.imageExtent.width);
				Util::SafeConvert(slice.m_Height, region.imageExtent.height);
				Util::SafeConvert(slice.m_Depth, region.imageExtent.depth);
			}

			currentOffset += slice.m_DataLength;
		}
	}

	// Copy staging buffer into destination texture
	{
		//auto uniqueCmdBuffer = queue.CreateCmdBufferAndBegin();
		auto& cmdBuffer = g_ShaderDevice.GetPrimaryCmdBuf();//*uniqueCmdBuffer;

		auto pixScope = cmdBuffer.DebugRegionBegin(PIX_COLOR_READWRITE, "ShaderAPI::UpdateTexture(%.*s)", PRINTF_SV(tex.GetDebugName()));

		cmdBuffer.TryEndRenderPass();

		const vk::PipelineStageFlags stageMask = vk::PipelineStageFlagBits::eTransfer;

		// TODO: Use stack allocation
		// TODO: Combine by mip level/layer
		std::vector<vk::ImageMemoryBarrier> barriers;
		for (size_t i = 0; i < count; i++)
		{
			auto& barrier = barriers.emplace_back();
			const auto& slice = data[i];

			barrier.image = tex.m_Image.GetImage();
			barrier.oldLayout = vk::ImageLayout::eUndefined; // Discard old contents
			barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			auto& srr = barrier.subresourceRange;
			srr.aspectMask = GetAspects(tex.m_CreateInfo.format);
			srr.baseMipLevel = slice.m_MipLevel;
			srr.baseArrayLayer = slice.m_CubeFace;
			srr.layerCount = 1;
			srr.levelCount = 1;
		}

		cmdBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			{}, {}, {}, barriers);

		cmdBuffer.copyBufferToImage(stagingBuf.GetBuffer(), tex.m_Image.GetImage(),
			vk::ImageLayout::eTransferDstOptimal, copyRegions);
		cmdBuffer.AddResource(std::move(stagingBuf));

		for (auto& barrier : barriers)
		{
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		}

		cmdBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{}, {}, {}, barriers);
	}

	return true;
}

void IShaderTextureManager::SetStandardTextureHandle(StandardTextureId_t id, ShaderAPITextureHandle_t tex)
{
	LOG_FUNC();
	m_StdTextures.at(id) = tex;
}

void IShaderTextureManager::TexLodClamp(int something)
{
	LOG_FUNC();

	if (something != 0)
		NOT_IMPLEMENTED_FUNC();
}

void IShaderTextureManager::TexLodBias(float bias)
{
	LOG_FUNC();

	if (bias != 0)
		NOT_IMPLEMENTED_FUNC();
}

void IShaderTextureManager::GetStandardTextureDimensions(int* width, int* height, StandardTextureId_t id)
{
	LOG_FUNC();

	auto& tex = GetTexture(m_StdTextures.at(id));
	auto& ci = tex.GetImageCreateInfo();

	if (width)
		Util::SafeConvert(ci.extent.width, *width);

	if (height)
		Util::SafeConvert(ci.extent.height, *height);
}
