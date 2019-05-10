#include "ResourceBlob.h"

using namespace TF2Vulkan;

struct ResourceBlob::BufferNode : IResource
{
	vk::UniqueBuffer m_Buffer;
};
struct ResourceBlob::ImageNode : IResource
{
	vk::UniqueImage m_Image;
};
struct ResourceBlob::ImageViewNode : IResource
{
	vk::UniqueImageView m_ImageView;
};
struct ResourceBlob::AllocatedBufferNode : IResource
{
	vma::AllocatedBuffer m_Buffer;
};
struct ResourceBlob::AllocatedImageNode : IResource
{
	vma::AllocatedImage m_Image;
};
struct ResourceBlob::DescriptorSetNode : IResource
{
	vk::UniqueDescriptorSet m_DescriptorSet;
};

void ResourceBlob::AddResource(vk::UniqueBuffer&& buffer)
{
	auto newNode = std::make_unique<BufferNode>();
	newNode->m_Buffer = std::move(buffer);
	AddResource(std::move(newNode));
}

void ResourceBlob::AddResource(vk::UniqueImage&& image)
{
	auto newNode = std::make_unique<ImageNode>();
	newNode->m_Image = std::move(image);
	AddResource(std::move(newNode));
}

void ResourceBlob::AddResource(vk::UniqueImageView&& imageView)
{
	auto newNode = std::make_unique<ImageViewNode>();
	newNode->m_ImageView = std::move(imageView);
	AddResource(std::move(newNode));
}

void ResourceBlob::AddResource(vma::AllocatedBuffer&& buffer)
{
	auto newNode = std::make_unique<AllocatedBufferNode>();
	newNode->m_Buffer = std::move(buffer);
	AddResource(std::move(newNode));
}

void ResourceBlob::AddResource(vma::AllocatedImage&& image)
{
	auto newNode = std::make_unique<AllocatedImageNode>();
	newNode->m_Image = std::move(image);
	AddResource(std::move(newNode));
}

void ResourceBlob::AddResource(vk::UniqueDescriptorSet&& descriptorSet)
{
	auto newNode = std::make_unique<DescriptorSetNode>();
	newNode->m_DescriptorSet = std::move(descriptorSet);
	AddResource(std::move(newNode));
}

void ResourceBlob::ReleaseAttachedResources()
{
	m_FirstNode.reset();
}

void ResourceBlob::AddResource(std::unique_ptr<IResource>&& node)
{
	node->m_Next = std::move(m_FirstNode);
	m_FirstNode = std::move(node);
}
