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

		BufferFactory& SetUsage(const vk::BufferUsageFlags& usage);
		BufferFactory& SetSize(size_t size);
		BufferFactory& SetInitialData(const void* initialData, size_t initialDataSize, size_t writeOffset = 0);
		BufferFactory& SetMemoryRequiredFlags(const vk::MemoryPropertyFlags& flags);
		BufferFactory& SetDebugName(std::string&& dbgName);

		vma::AllocatedBuffer Create() const;

		vk::BufferUsageFlags m_Usage{};
		vk::MemoryPropertyFlags m_MemoryRequiredFlags;
		size_t m_Size = 0;
		const void* m_InitialData = nullptr;
		size_t m_InitialDataSize = 0;
		size_t m_InitialDataWriteOffset = 0;
	};

} }
