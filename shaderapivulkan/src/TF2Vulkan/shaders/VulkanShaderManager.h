#pragma once

#include <TF2Vulkan/Util/utlsymbol.h>

namespace spirv_cross
{
	class Compiler;
	struct Resource;
}

class IMaterial;

namespace TF2Vulkan
{
	namespace ShaderReflection
	{
		enum class VariableType : uint_fast8_t
		{
			Invalid = uint_fast8_t(-1),

			Void = 0,
			Boolean,
			SByte,
			UByte,
			Short,
			Int,
			UInt,
			Int64,
			UInt64,
			Half,
			Float,
			Double,
			Struct,
			Image,
			SampledImage,
			Sampler,
		};

		struct ShaderVariable
		{
			ShaderVariable(const spirv_cross::Compiler& comp, uint32_t id);
			explicit ShaderVariable(std::string&& name);

			std::string m_Name;
			VariableType m_Type;
			uint_fast8_t m_ComponentCount; // vector size (1/2/3/4 elements)
		};

		struct SpecializationConstant final : ShaderVariable
		{
			SpecializationConstant(const spirv_cross::Compiler& comp, uint32_t id, uint32_t constID);

			uint32_t m_ConstantID;
		};

		struct VertexAttribute final : ShaderVariable
		{
			VertexAttribute(const spirv_cross::Compiler& comp, uint32_t id);

			std::string m_Semantic;
			uint32_t m_Location;
		};

		struct StructMember final : ShaderVariable
		{
			StructMember(const spirv_cross::Compiler& comp, uint32_t parent, uint32_t index);

			uint32_t m_Parent;
			uint32_t m_Offset;
		};

		struct Struct
		{
			Struct(const spirv_cross::Compiler& comp,
				const spirv_cross::Resource& resource);

			std::string m_Name;
			size_t m_Size;
			std::vector<StructMember> m_Members;
		};

		// Anything that has a binding (cbufs, samplers, textures, etc)
		struct ShaderResource
		{
			ShaderResource(const spirv_cross::Compiler& comp, uint32_t id);

			uint32_t m_Binding;
		};

		struct UniformBuffer final : Struct, ShaderResource
		{
			UniformBuffer(const spirv_cross::Compiler& comp,
				const spirv_cross::Resource& resource);
		};

		struct Sampler final : ShaderVariable, ShaderResource
		{
			Sampler(const spirv_cross::Compiler& comp, uint32_t id);
		};

		struct Texture final : ShaderVariable, ShaderResource
		{
			Texture(const spirv_cross::Compiler& comp, uint32_t id);
		};

		struct ReflectionData final
		{
			vk::ShaderStageFlags m_ShaderStage;

			std::vector<SpecializationConstant> m_SpecConstants;
			std::vector<VertexAttribute> m_VertexInputs;
			std::vector<VertexAttribute> m_VertexOutputs;
			std::vector<UniformBuffer> m_UniformBuffers;
			std::vector<Sampler> m_Samplers;
			std::vector<Texture> m_Textures;
		};
	}

	class IVulkanShader
	{
	public:
		virtual vk::ShaderModule GetModule() const = 0;
		virtual const CUtlSymbolDbg& GetName() const = 0;
		virtual const ShaderReflection::ReflectionData& GetReflectionData() const = 0;

	protected:
		virtual ~IVulkanShader() = default;
	};

	class IVulkanShaderManager
	{
	public:
		virtual ~IVulkanShaderManager() = default;

		virtual const IVulkanShader& FindOrCreateShader(const CUtlSymbolDbg& id) = 0;
	};

	extern IVulkanShaderManager& g_ShaderManager;
}
