#include "TF2Vulkan/ShaderDevice.h"
#include "VulkanShaderManager.h"

#include <TF2Vulkan/Util/std_utility.h>
#include <stdshader_dx9_tf2vulkan/ShaderBlobs.h>

#include <materialsystem/shader_vcs_version.h>

#include <spirv_cross.hpp>

#include <mutex>
#include <unordered_map>

using namespace TF2Vulkan;
using namespace TF2Vulkan::ShaderReflection;

STD_HASH_DEFINITION(TF2Vulkan::IVulkanShaderManager::ShaderID,
	v.m_Name,
	v.m_StaticIndex
);

namespace
{
	class VulkanShaderManager final : public IVulkanShaderManager
	{
	public:
		const IShader& FindOrCreateShader(const ShaderID& id) override;

	private:

		struct CompiledShader final : IShader
		{
			CompiledShader(const ShaderID& id);
			vk::ShaderModule GetModule() const override { return m_Shader.get(); }
			const ShaderID& GetID() const override { return *m_ID; }
			const ReflectionData& GetReflectionData() const override { return m_ReflectionData; }

			const ShaderID* m_ID = nullptr;
			vk::UniqueShaderModule m_Shader;
			ReflectionData m_ReflectionData;
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

auto VulkanShaderManager::FindOrCreateShader(const ShaderID& id) -> const IShader&
{
	std::lock_guard lock(m_Mutex);

	assert(id.m_StaticIndex == 0);

	if (auto found = m_Shaders.find(id); found != m_Shaders.end())
		return found->second;

	// Couldn't find an existing one, we need to create it here
	return m_Shaders.emplace(id, id).first->second;
}

static ReflectionData CreateReflectionData(const void* data, size_t byteSize)
{
	ReflectionData retVal;

	const spirv_cross::Compiler comp(reinterpret_cast<const uint32_t*>(data), byteSize);

	// Specialization constants
	for (const auto& specConstIn : comp.get_specialization_constants())
	{
		auto& specConstOut = retVal.m_SpecConstants.emplace_back();
		specConstOut.m_ID = specConstIn.constant_id;
		specConstOut.m_Name = comp.get_name(specConstIn.id);
	}

	return retVal;
}

VulkanShaderManager::CompiledShader::CompiledShader(const ShaderID& id) :
	m_ID(&id)
{
	vk::ShaderModuleCreateInfo ci;

	const auto blobType = s_ShaderBlobMapping.at(id.m_Name.String());

	const void* blobData;
	if (!TF2Vulkan::GetShaderBlob(blobType, blobData, ci.codeSize))
		throw VulkanException("Failed to get shader blob", EXCEPTION_DATA());

	ci.pCode = reinterpret_cast<const uint32_t*>(blobData);

	m_Shader = g_ShaderDevice.GetVulkanDevice().createShaderModuleUnique(ci);
	g_ShaderDevice.SetDebugName(m_Shader, id.m_Name.String());

	m_ReflectionData = CreateReflectionData(blobData, ci.codeSize / sizeof(uint32_t));
}
