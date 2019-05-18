#pragma once

namespace TF2Vulkan{ namespace Factories
{
	template<typename T>
	struct FactoryBase
	{
		T& SetDebugName(std::string&& dbgName);
		T& SetDebugName(const std::string_view& dbgName);

		std::string m_DebugName;
	};

	struct BufferFactory : FactoryBase<BufferFactory>
	{
		BufferFactory() = default;

		BufferFactory& SetAllowMapping(bool allow = true);
		BufferFactory& SetUsage(const vk::BufferUsageFlags& usage);
		BufferFactory& SetSize(size_t size);
		BufferFactory& SetInitialData(const void* initialData, size_t initialDataSize, size_t writeOffset = 0);
		BufferFactory& SetMemoryRequiredFlags(const vk::MemoryPropertyFlags& flags);
		BufferFactory& SetDebugName(std::string&& dbgName);

		vma::AllocatedBuffer Create() const;

		vk::BufferCreateInfo m_CreateInfo;
		vma::AllocationCreateInfo m_AllocInfo;

		const void* m_InitialData = nullptr;
		size_t m_InitialDataSize = 0;
		size_t m_InitialDataWriteOffset = 0;
	};

	struct ImageFactory : FactoryBase<ImageFactory>
	{
		ImageFactory() = default;

		ImageFactory& SetMemoryUsage(VmaMemoryUsage usage);
		ImageFactory& SetCreateInfo(const vk::ImageCreateInfo& createInfo);

		vma::AllocatedImage Create() const;

		vk::ImageCreateInfo m_CreateInfo;
		vma::AllocationCreateInfo m_AllocInfo;
	};
} }
