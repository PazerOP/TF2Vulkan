#pragma once

#include <TF2Vulkan/Util/utlsymbol.h>

namespace TF2Vulkan
{
	namespace ShaderReflection
	{
		struct SpecializationConstant final
		{
			std::string m_Name;
			uint32_t m_ID;
		};

		struct VertexAttribute final
		{
			std::string m_Semantic;
			std::string m_Name;
			uint32_t m_Location;
		};

		struct ReflectionData final
		{
			std::vector<SpecializationConstant> m_SpecConstants;
			std::vector<VertexAttribute> m_VertexAttributes;
		};
	}

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

		class IShader
		{
		public:
			virtual vk::ShaderModule GetModule() const = 0;
			virtual const ShaderID& GetID() const = 0;
			virtual const ShaderReflection::ReflectionData& GetReflectionData() const = 0;

		protected:
			virtual ~IShader() = default;
		};

		virtual const IShader& FindOrCreateShader(const ShaderID& id) = 0;

		const IShader& FindOrCreateShader(const CUtlSymbolDbg& name, int staticIndex)
		{
			return FindOrCreateShader({ name, staticIndex });
		}
	};

	extern IVulkanShaderManager& g_ShaderManager;
}
