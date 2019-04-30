#pragma once

#include <TF2Vulkan/Util/utlsymbol.h>

namespace TF2Vulkan
{
	class IVulkanShaderManager
	{
	public:
		virtual ~IVulkanShaderManager() = default;

		struct ShaderID
		{
			CUtlSymbolDbg m_Name;
			int m_StaticIndex = 0;

			DEFAULT_STRONG_EQUALITY_OPERATOR(ShaderID);
		};

		virtual vk::ShaderModule FindOrCreateShader(const ShaderID& id) = 0;

		vk::ShaderModule FindOrCreateShader(const CUtlSymbolDbg& name, int staticIndex)
		{
			return FindOrCreateShader({ name, staticIndex });
		}
	};

	extern IVulkanShaderManager& g_ShaderManager;
}
