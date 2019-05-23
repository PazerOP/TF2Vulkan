#include "ResourceBlob.h"

using namespace TF2Vulkan;

void ResourceBlob::AddResource(vk::UniqueBuffer&& buffer)
{
	m_Resources.emplace_back(std::move(buffer));
}

void ResourceBlob::AddResource(vk::UniqueImage&& image)
{
	m_Resources.emplace_back(std::move(image));
}

void ResourceBlob::AddResource(vk::UniqueImageView&& imageView)
{
	m_Resources.emplace_back(std::move(imageView));
}

void ResourceBlob::AddResource(vma::AllocatedBuffer&& buffer)
{
	m_Resources.emplace_back(std::move(buffer));
}

void ResourceBlob::AddResource(vma::AllocatedImage&& image)
{
	m_Resources.emplace_back(std::move(image));
}

void ResourceBlob::AddResource(vk::UniqueDescriptorSet&& descriptorSet)
{
	m_Resources.emplace_back(std::move(descriptorSet));
}

void ResourceBlob::ReleaseAttachedResources()
{
	m_Resources.clear();
}
