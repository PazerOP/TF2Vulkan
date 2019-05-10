#pragma once

namespace TF2Vulkan
{
	class ResourceBlob
	{
	public:
		void AddResource(vk::UniqueBuffer&& buffer);
		void AddResource(vk::UniqueImage&& image);
		void AddResource(vk::UniqueImageView&& imageView);
		void AddResource(vma::AllocatedBuffer&& buffer);
		void AddResource(vma::AllocatedImage&& image);
		void AddResource(vk::UniqueDescriptorSet&& descriptor);

		template<typename TContainer>
		auto AddResource(TContainer&& container) -> decltype(std::begin(container), std::end(container), AddResource(std::move(*std::begin(container))))
		{
			for (auto&& val : container)
				AddResource(std::move(val));
		}

	protected:
		void ReleaseAttachedResources();

	private:
		struct BufferNode;
		struct ImageNode;
		struct ImageViewNode;
		struct AllocatedBufferNode;
		struct AllocatedImageNode;
		struct DescriptorSetNode;

		struct IResource
		{
			virtual ~IResource() = default;
			std::unique_ptr<IResource> m_Next;
		};

		void AddResource(std::unique_ptr<IResource>&& node);

		std::unique_ptr<IResource> m_FirstNode;
	};
}
