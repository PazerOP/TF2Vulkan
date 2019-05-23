#pragma once

#include <variant>

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
		struct IResource
		{
			virtual ~IResource() = default;
		};

		using ResourceTypes = std::variant<
			std::unique_ptr<IResource>,
			vk::UniqueBuffer,
			vk::UniqueImage,
			vk::UniqueImageView,
			vma::AllocatedBuffer,
			vma::AllocatedImage,
			vk::UniqueDescriptorSet>;

		std::vector<ResourceTypes> m_Resources;
	};
}
