#include "TF2Vulkan/FormatInfo.h"
#include "TextureData.h"

using namespace TF2Vulkan;

template<bool assertEnabled> static bool Validate(const TextureData& data)
{
	if (data.m_Width < 1)
	{
		assert(!assertEnabled);
		return false;
	}
	if (data.m_Height < 1)
	{
		assert(!assertEnabled);
		return false;
	}
	if (data.m_Depth < 1)
	{
		assert(!assertEnabled);
		return false;
	}

	if (data.m_Format == ImageFormat::IMAGE_FORMAT_UNKNOWN)
	{
		assert(!assertEnabled);
		return false;
	}

	if (!data.m_Data)
	{
		assert(!assertEnabled);
		return false;
	}

	if (auto minMemReq = Util::SafeConvert<size_t>(ImageLoader::GetMemRequired(data.m_Width, data.m_Height, data.m_Depth, data.m_Format, false));
		data.m_DataLength < minMemReq)
	{
		assert(!assertEnabled);
		return false;
	}

	// Verify that image isn't "empty" depth-wise
	if (data.m_Depth > 1 && data.m_SliceStride <= 0)
	{
		assert(!assertEnabled);
		return false;
	}

	// Verify that width/height are a multiple of block size
	{
		auto blockSize = FormatInfo::GetBlockSize(data.m_Format);
		if ((data.m_Width % blockSize.width) || (data.m_Height % blockSize.height))
		{
			assert(!assertEnabled);
			return false;
		}
	}

	return true;
}

bool TextureData::Validate() const
{
	return ::Validate<true>(*this);
}

bool TextureData::ValidateNoAssert() const
{
	return ::Validate<false>(*this);
}
