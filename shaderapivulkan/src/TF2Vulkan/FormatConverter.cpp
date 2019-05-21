#include "stdafx.h"
#include "FormatConverter.h"

#undef min
#undef max

using namespace TF2Vulkan;

#define DECLARE_COMPONENT_HELPERS(type, index, bits, compNameLower, compNameUpper) \
	static constexpr bool HAS_ ## compNameUpper = true; \
	static constexpr bool HAS_ ## index = true; \
	static constexpr auto INDEX_ ## compNameUpper = index; \
	static constexpr auto COMPONENT_BITS_ ## compNameUpper = bits; \
	using ChannelType ## componentNameUpper = type;

#define DECLARE_COMPONENT(type, index, compNameLower, compNameUpper) \
	DECLARE_COMPONENT_HELPERS(type, index, sizeof(type) * CHAR_BIT, compNameLower, compNameUpper); \
	type compNameLower;

#define DECLARE_R(type, index) DECLARE_COMPONENT(type, index, r, R)
#define DECLARE_G(type, index) DECLARE_COMPONENT(type, index, g, G)
#define DECLARE_B(type, index) DECLARE_COMPONENT(type, index, b, B)
#define DECLARE_A(type, index) DECLARE_COMPONENT(type, index, a, A)

namespace
{
	struct PixelFormatBase
	{
		static constexpr bool HAS_R = false;
		static constexpr bool HAS_G = false;
		static constexpr bool HAS_B = false;
		static constexpr bool HAS_A = false;

		static constexpr bool HAS_0 = false;
		static constexpr bool HAS_1 = false;
		static constexpr bool HAS_2 = false;
		static constexpr bool HAS_3 = false;

		static constexpr auto INDEX_R = -1;
		static constexpr auto INDEX_G = -1;
		static constexpr auto INDEX_B = -1;
		static constexpr auto INDEX_A = -1;

		static constexpr auto CHANNEL_BITS_R = 0;
		static constexpr auto CHANNEL_BITS_G = 0;
		static constexpr auto CHANNEL_BITS_B = 0;
		static constexpr auto CHANNEL_BITS_A = 0;

		static constexpr uint_fast8_t DEFAULT_R = 0;
		static constexpr uint_fast8_t DEFAULT_G = 0;
		static constexpr uint_fast8_t DEFAULT_B = 0;
		static constexpr uint_fast8_t DEFAULT_A = 255;

		using ChannelTypeR = uint8_t;
		using ChannelTypeG = uint8_t;
		using ChannelTypeB = uint8_t;
		using ChannelTypeA = uint8_t;
	};

	template<ImageFormat fmt> struct PixelFormatData;

	template<> struct PixelFormatData<IMAGE_FORMAT_RGB888> : PixelFormatBase
	{
		DECLARE_R(uint8_t, 0);
		DECLARE_G(uint8_t, 1);
		DECLARE_B(uint8_t, 2);
	};
	template<> struct PixelFormatData<IMAGE_FORMAT_RGBA8888> : PixelFormatBase
	{
		DECLARE_R(uint8_t, 0);
		DECLARE_G(uint8_t, 1);
		DECLARE_B(uint8_t, 2);
		DECLARE_A(uint8_t, 3);
	};
	template<> struct PixelFormatData<IMAGE_FORMAT_ARGB8888> : PixelFormatBase
	{
		DECLARE_A(uint8_t, 0);
		DECLARE_R(uint8_t, 1);
		DECLARE_G(uint8_t, 2);
		DECLARE_B(uint8_t, 3);
	};

	template<> struct PixelFormatData<IMAGE_FORMAT_BGR888> : PixelFormatBase
	{
		DECLARE_B(uint8_t, 0);
		DECLARE_G(uint8_t, 1);
		DECLARE_R(uint8_t, 2);
	};
	template<> struct PixelFormatData<IMAGE_FORMAT_BGRX8888> : PixelFormatBase
	{
		DECLARE_B(uint8_t, 0);
		DECLARE_G(uint8_t, 1);
		DECLARE_R(uint8_t, 2);
		uint8_t : 8; // Unused padding
	};
	template<> struct PixelFormatData<IMAGE_FORMAT_BGRA8888> : PixelFormatBase
	{
		DECLARE_B(uint8_t, 0);
		DECLARE_G(uint8_t, 1);
		DECLARE_R(uint8_t, 2);
		DECLARE_A(uint8_t, 3);
	};
	template<> struct PixelFormatData<IMAGE_FORMAT_ABGR8888> : PixelFormatBase
	{
		DECLARE_A(uint8_t, 0);
		DECLARE_B(uint8_t, 1);
		DECLARE_G(uint8_t, 2);
		DECLARE_R(uint8_t, 3);
	};

	enum class ImageChannel : uint_fast8_t
	{
		R,
		G,
		B,
		A
	};

#define VALIDATE_IMAGECHANNEL(channel) \
	static_assert((channel) == ImageChannel::R || (channel) == ImageChannel::G || (channel) == ImageChannel::B || (channel) == ImageChannel::A);

	struct MaxValueType final {};
	static constexpr MaxValueType MAX_VALUE;
	struct MinValueType final {};
	static constexpr MinValueType MIN_VALUE;

	template<ImageFormat fmt>
	struct PixelFormat : PixelFormatData<fmt>
	{
		using DataType = PixelFormatData<fmt>;
	private:
		template<typename T, size_t bits> static constexpr auto GetMaxValue()
		{
			if constexpr (std::is_arithmetic_v<T>)
			{
				if constexpr (std::is_floating_point_v<T> || bits == (sizeof(T) * CHAR_BIT))
					return std::numeric_limits<T>::max();
				else
					return T((1ULL << bits) - 1);
			}
			else
			{
				return 0;
			}
		}

	public:
		template<ImageChannel channel> static constexpr auto GetChannelIndex()
		{
			VALIDATE_IMAGECHANNEL(channel);

			if constexpr (channel == ImageChannel::R)
				return DataType::INDEX_R;
			else if constexpr (channel == ImageChannel::G)
				return DataType::INDEX_G;
			else if constexpr (channel == ImageChannel::B)
				return DataType::INDEX_B;
			else if constexpr (channel == ImageChannel::A)
				return DataType::INDEX_A;
		}

		template<ImageChannel channel> static constexpr auto GetChannelBits()
		{
			VALIDATE_IMAGECHANNEL(channel);

			if constexpr (channel == ImageChannel::R)
				return DataType::CHANNEL_BITS_R;
			else if constexpr (channel == ImageChannel::G)
				return DataType::CHANNEL_BITS_G;
			else if constexpr (channel == ImageChannel::B)
				return DataType::CHANNEL_BITS_B;
			else if constexpr (channel == ImageChannel::A)
				return DataType::CHANNEL_BITS_A;
		}

		template<ImageChannel channel> static constexpr auto GetChannelType()
		{
			VALIDATE_IMAGECHANNEL(channel);

			if constexpr (channel == ImageChannel::R)
			{
				if constexpr (DataType::HAS_R)
					return typename DataType::ChannelTypeR{};
				else
					return;
			}
			else if constexpr (channel == ImageChannel::G)
			{
				if constexpr (DataType::HAS_G)
					return typename DataType::ChannelTypeG{};
				else
					return;
			}
			else if constexpr (channel == ImageChannel::B)
			{
				if constexpr (DataType::HAS_B)
					return typename DataType::ChannelTypeB{};
				else
					return;
			}
			else if constexpr (channel == ImageChannel::A)
			{
				if constexpr (DataType::HAS_A)
					return typename DataType::ChannelTypeA{};
				else
					return;
			}
		}

		template<ImageChannel channel> static constexpr auto GetChannelMin()
		{
			return 0;
		}

		template<ImageChannel channel> static constexpr auto GetChannelMax()
		{
			return GetMaxValue<decltype(GetChannelType<channel>()), GetChannelBits<channel>()>();
		}

		template<ImageChannel channel> constexpr auto GetChannelValue() const
		{
			VALIDATE_IMAGECHANNEL(channel);

			if constexpr (channel == ImageChannel::R)
			{
				if constexpr (DataType::HAS_R)
					return DataType::r;
				else
					return MIN_VALUE;
			}
			else if constexpr (channel == ImageChannel::G)
			{
				if constexpr (DataType::HAS_G)
					return DataType::g;
				else
					return MIN_VALUE;
			}
			else if constexpr (channel == ImageChannel::B)
			{
				if constexpr (DataType::HAS_B)
					return DataType::b;
				else
					return MIN_VALUE;
			}
			else if constexpr (channel == ImageChannel::A)
			{
				if constexpr (DataType::HAS_A)
					return DataType::a;
				else
					return MAX_VALUE;
			}
		}

		template<ImageChannel channel, typename T>
		constexpr void TrySetChannelValue(const T& value)
		{
			if constexpr (std::is_same_v<T, MaxValueType>)
			{
				return TrySetChannelValue<channel>(GetChannelMax<channel>());
			}
			else
			{
				VALIDATE_IMAGECHANNEL(channel);

				if constexpr (channel == ImageChannel::R)
				{
					if constexpr (DataType::HAS_R)
						DataType::r = value;
				}
				else if constexpr (channel == ImageChannel::G)
				{
					if constexpr (DataType::HAS_G)
						DataType::g = value;
				}
				else if constexpr (channel == ImageChannel::B)
				{
					if constexpr (DataType::HAS_B)
						DataType::b = value;
				}
				else if constexpr (channel == ImageChannel::A)
				{
					if constexpr (DataType::HAS_A)
						DataType::a = value;
				}
			}
		}
	};
}

template<ImageChannel channel, ImageFormat srcFormat, ImageFormat dstFormat>
static void ConvertChannel(const PixelFormat<srcFormat>& RESTRICT src, PixelFormat<dstFormat>& RESTRICT dst)
{
	dst.TrySetChannelValue<channel>(src.GetChannelValue<channel>());
}

template<ImageFormat srcFormat, ImageFormat dstFormat>
static void ConvertImpl(const std::byte* RESTRICT srcRaw, size_t srcSize,
	std::byte* RESTRICT dstRaw, size_t dstSize,
	uint32_t width, uint32_t height, size_t srcStride, size_t dstStride)
{
	using SrcType = PixelFormat<srcFormat>;
	using DstType = PixelFormat<dstFormat>;

	if (dstStride == 0)
		Util::SafeConvert(ImageLoader::GetMemRequired(width, 1, 1, dstFormat, false), dstStride);

	const auto* RESTRICT srcPtr = reinterpret_cast<const SrcType*>(srcRaw);
	auto* RESTRICT dstPtr = reinterpret_cast<DstType*>(dstRaw);

	assert(srcStride >= sizeof(SrcType) * width);
	assert(dstStride >= sizeof(DstType) * width);

	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < height; x++)
		{
			const auto& RESTRICT src = srcPtr[x];
			auto& RESTRICT dst = dstPtr[x];

			ConvertChannel<ImageChannel::R>(src, dst);
			ConvertChannel<ImageChannel::G>(src, dst);
			ConvertChannel<ImageChannel::B>(src, dst);
			ConvertChannel<ImageChannel::A>(src, dst);
		}

		*reinterpret_cast<const std::byte**>(&srcPtr) += srcStride;
		*reinterpret_cast<std::byte**>(&dstPtr) += dstStride;
	}

	assert(reinterpret_cast<const std::byte*>(srcPtr) <= (srcRaw + srcSize));
	assert(reinterpret_cast<const std::byte*>(dstPtr) <= (dstRaw + dstSize));
}

void FormatConverter::Convert(
	const std::byte* src, ImageFormat srcFormat, size_t srcSize,
	std::byte* dst, ImageFormat dstFormat, size_t dstSize,
	uint32_t width, uint32_t height, size_t srcStride, size_t dstStride)
{
	assert(srcSize > 0);
	assert(dstSize > 0);

	ImageFormatVisit(srcFormat, [&](const auto & srcFormatType)
		{
			ImageFormatVisit(dstFormat, [&](const auto & dstFormatType)
				{
					ConvertImpl<srcFormatType.FORMAT, dstFormatType.FORMAT>(src, srcSize, dst, dstSize,
						width, height, srcStride, dstStride);
				});
		});
}
