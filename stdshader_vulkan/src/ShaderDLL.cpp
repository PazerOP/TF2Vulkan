#include "IShaderSystem.h"
#include "shaders/BaseShaderNext.h"

#include <TF2Vulkan/IShaderDynamicNext.h>
#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/SafeConvert.h>
#include <TF2Vulkan/Util/std_algorithm.h>

#include <materialsystem/IShader.h>
#include <materialsystem/materialsystem_config.h>
#include <shaderlib/ShaderDLL.h>

#include <tier0/icommandline.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>
#include <tier3/tier3.h>

#include <cstdint>
#include <vector>

using namespace TF2Vulkan;

namespace
{
	class VulkanShaderDLL final : public IShaderDLLInternal, public IShaderDLL
	{
		// Inherited via IShaderDLLInternal
		bool Connect(CreateInterfaceFn factory, bool isMaterialSystem) override;
		void Disconnect(bool isMaterialSystem) override { NOT_IMPLEMENTED_FUNC(); }
		int ShaderCount() const override;
		IShader* GetShader(int index) override;

		// Inherited via IShaderDLL
		void InsertShader(IShader* pShader) override;

		std::vector<IShader*> m_Shaders;
	};
}

IShaderSystem* g_ShaderSystem;
IMaterialSystemHardwareConfig* g_pHardwareConfig;
const MaterialSystem_Config_t* g_pConfig;
IShaderDynamicNext* TF2Vulkan::g_ShaderDynamic;
IShaderNextFactory* TF2Vulkan::g_ShaderFactory;

static VulkanShaderDLL& GetShaderDLLRef()
{
	static VulkanShaderDLL s_VulkanShaderDLL;
	return s_VulkanShaderDLL;
}

IShaderDLLInternal* GetShaderDLLInternal()
{
	return &GetShaderDLLRef();
}
IShaderDLL* GetShaderDLL()
{
	return &GetShaderDLLRef();
}

EXPOSE_INTERFACE_FN([] { return (void*)GetShaderDLLInternal(); }, IShaderDLLInternal, SHADER_DLL_INTERFACE_VERSION);

bool VulkanShaderDLL::Connect(CreateInterfaceFn factory, bool isMaterialSystem)
{
	LOG_FUNC();

	ConnectTier1Libraries(&factory, 1);
	ConnectTier2Libraries(&factory, 1);
	ConnectTier3Libraries(&factory, 1);

	Util::ConnectInterface(factory, SHADERSYSTEM_INTERFACE_VERSION, g_ShaderSystem);
	Util::ConnectInterface(factory, MATERIALSYSTEM_CONFIG_VERSION, g_pConfig);
	Util::ConnectInterface(factory, MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, g_pHardwareConfig);

	auto shaderapiFactory = Sys_GetFactory("shaderapidx9");
	Util::ConnectInterface(shaderapiFactory, SHADERDYNAMICNEXT_INTERFACE_VERSION, g_ShaderDynamic);
	Util::ConnectInterface(shaderapiFactory, SHADERNEXTFACTORY_INTERFACE_VERSION, g_ShaderFactory);

	if (!CommandLine()->CheckParm("-insecure"))
	{
		Error("Attempted to use stdshader_vulkan.dll without -insecure");
		return false;
	}

	// Initialize all shaders
	for (const auto& shader : m_Shaders)
	{
		if (auto nShader = dynamic_cast<Shaders::BaseShaderNext*>(shader))
			nShader->InitShader(*g_ShaderFactory);
	}

	return true;
}

void VulkanShaderDLL::InsertShader(IShader* shader)
{
	LOG_FUNC();

	// Make sure there aren't any duplicates
	assert(!Util::algorithm::contains_if(m_Shaders, [&](const IShader * s)
		{ return !strcmp(s->GetName(), shader->GetName()); }));

	m_Shaders.push_back(shader);
}

int VulkanShaderDLL::ShaderCount() const
{
	LOG_FUNC();
	return Util::SafeConvert<int>(m_Shaders.size());
}

IShader* VulkanShaderDLL::GetShader(int index)
{
	LOG_FUNC();
	return m_Shaders.at(Util::SafeConvert<size_t>(index));
}
