#include "TF2Vulkan/ShaderDevice.h"
#include "VulkanShaderManager.h"

#include <TF2Vulkan/Util/std_utility.h>
#include <stdshader_dx9_tf2vulkan/ShaderBlobs.h>

#include <materialsystem/shader_vcs_version.h>

#include <mutex>
#include <unordered_map>

using namespace TF2Vulkan;

STD_HASH_DEFINITION(TF2Vulkan::IVulkanShaderManager::ShaderID,
	v.m_Name,
	v.m_StaticIndex
);

namespace
{
	class VulkanShaderManager final : public IVulkanShaderManager
	{
	public:
		vk::ShaderModule FindOrCreateShader(const ShaderID& id) override;

	private:
		struct CompiledShader
		{
			vk::UniqueShaderModule m_Shader;
		};

		std::recursive_mutex m_Mutex;
		std::unordered_map<ShaderID, CompiledShader> m_Shaders;
	};
}

static VulkanShaderManager s_ShaderManager;
IVulkanShaderManager& TF2Vulkan::g_ShaderManager = s_ShaderManager;

static const std::unordered_map<std::string_view, ShaderBlob> s_ShaderBlobMapping =
{
	{ "vertexlit_and_unlit_generic_vs30", ShaderBlob::VertexLitAndUnlitGeneric_VS },
	{ "vertexlit_and_unlit_generic_ps30", ShaderBlob::VertexLitAndUnlitGeneric_PS },
};

vk::ShaderModule VulkanShaderManager::FindOrCreateShader(const ShaderID& id)
{
	std::lock_guard lock(m_Mutex);

	assert(id.m_StaticIndex == 0);

	if (auto found = m_Shaders.find(id); found != m_Shaders.end())
		return found->second.m_Shader.get();

	// Couldn't find an existing one, we need to create it here
	vk::ShaderModuleCreateInfo ci;

	const auto blobType = s_ShaderBlobMapping.at(id.m_Name.String());

	const void* blobData;
	if (!TF2Vulkan::GetShaderBlob(blobType, blobData, ci.codeSize))
		throw VulkanException("Failed to get shader blob", EXCEPTION_DATA());

	ci.pCode = reinterpret_cast<const uint32_t*>(blobData);

	auto createdUnique = g_ShaderDevice.GetVulkanDevice().createShaderModuleUnique(ci);
	g_ShaderDevice.SetDebugName(createdUnique, id.m_Name.String());

	vk::ShaderModule created = createdUnique.get();
	m_Shaders[id] = { std::move(createdUnique) };
	return created;
}
