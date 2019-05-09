#include "interface/internal/IShaderDeviceInternal.h"
#include "VulkanShaderManager.h"

#include <TF2Vulkan/Util/Buffer.h>
#include <TF2Vulkan/Util/std_utility.h>
#include <stdshader_dx9_tf2vulkan/ShaderBlobs.h>

#include <materialsystem/shader_vcs_version.h>

#include <spirv_cross.hpp>

#include <mutex>
#include <unordered_map>

using namespace TF2Vulkan;
using namespace TF2Vulkan::ShaderReflection;

namespace
{
	struct SpecConstMapping final
	{
		const char* m_SpecConstName;
		uint_fast8_t m_ComboOffset;
		uint32_t m_ComboMask = 0x1;
	};

	static constexpr SpecConstMapping s_SpecConstMap_VertexLitGeneric_VS[] =
	{
		{ "VERTEXCOLOR", 8, },
		{ "CUBEMAP", 9 },
		{ "HALFLAMBERT", 10 },
		{ "FLASHLIGHT", 11 },
		{ "SEAMLESS_BASE", 12 },
		{ "SEAMLESS_DETAIL", 13 },
		{ "SEPARATE_DETAIL_UVS", 14 },
		{ "DONT_GAMMA_CONVERT_VERTEX_COLOR", 16 },
	};

	struct ShaderInfo
	{
		constexpr ShaderInfo(ShaderBlob blob) : m_Blob(blob) {}
		template<size_t size>
		constexpr ShaderInfo(ShaderBlob blob, const SpecConstMapping(&specConstMappings)[size]) :
			m_Blob(blob), m_SpecConstMappings(specConstMappings), m_SpecConstMappingsCount(size)
		{
		}

		ShaderBlob m_Blob;
		const SpecConstMapping* m_SpecConstMappings = nullptr;
		size_t m_SpecConstMappingsCount = 0;
	};

	class VulkanShaderManager final : public IVulkanShaderManager
	{
	public:
		const IShader& FindOrCreateShader(const CUtlSymbolDbg& name) override;

	private:
		struct CompiledShader final : IShader
		{
			CompiledShader(const CUtlSymbolDbg& name);
			vk::ShaderModule GetModule() const override { return m_Shader.get(); }
			const CUtlSymbolDbg& GetName() const override { return m_Name; }
			const ReflectionData& GetReflectionData() const override { return m_ReflectionData; }
			void CreateSpecializationInfo(uint32_t combo, vk::SpecializationInfo& info,
				std::vector<vk::SpecializationMapEntry>& entries, std::vector<std::byte>& data) const override;

			CUtlSymbolDbg m_Name;
			vk::UniqueShaderModule m_Shader;
			ReflectionData m_ReflectionData;
			const ShaderInfo* m_Info;
		};

		std::recursive_mutex m_Mutex;
		std::unordered_map<CUtlSymbolDbg, CompiledShader> m_Shaders;
	};

	union vertexlit_and_unlit_generic_vs30_static_index
	{
		uint32_t m_Index;
		struct
		{
			uint32_t : 1;                                  // 0
			uint32_t : 1;                                  // 1
			uint32_t : 1;                                  // 2
			uint32_t : 1;                                  // 3
			uint32_t : 1;                                  // 4
			uint32_t : 1;                                  // 5
			uint32_t : 1;                                  // 6
			uint32_t : 1;                                  // 7
			uint32_t VERTEXCOLOR : 1;                      // 8
			uint32_t CUBEMAP : 1;                          // 9
			uint32_t HALFLAMBERT : 1;                      // 10
			uint32_t FLASHLIGHT : 1;                       // 11
			uint32_t SEAMLESS_BASE : 1;                    // 12
			uint32_t SEAMLESS_DETAIL : 1;                  // 13
			uint32_t SEPARATE_DETAIL_UVS : 1;              // 14
			uint32_t USE_STATIC_CONTROL_FLOW : 1;          // 15
			uint32_t DONT_GAMMA_CONVERT_VERTEX_COLOR : 1;  // 16
		};
	};
	static_assert(sizeof(vertexlit_and_unlit_generic_vs30_static_index) == sizeof(uint32_t));

	struct vertexlit_and_unlit_generic_vs30_dynamic_index
	{
		uint32_t m_Index;
		union
		{
			uint32_t : 1;              // 0
			uint32_t : 1;              // 1
			uint32_t : 1;              // 2
			uint32_t : 1;              // 3
			uint32_t : 1;              // 4
			uint32_t : 1;              // 5
			uint32_t : 1;              // 6
		};
	};
}

static VulkanShaderManager s_ShaderManager;
IVulkanShaderManager& TF2Vulkan::g_ShaderManager = s_ShaderManager;

static const std::unordered_map<std::string_view, ShaderInfo> s_ShaderBlobMapping =
{
	{ "bik_vs20", ShaderBlob::Bik_VS },
	{ "bik_ps20b", ShaderBlob::Bik_PS },
	{ "vertexlit_and_unlit_generic_vs30", { ShaderBlob::VertexLitAndUnlitGeneric_VS, s_SpecConstMap_VertexLitGeneric_VS } },
	{ "vertexlit_and_unlit_generic_ps30", ShaderBlob::VertexLitAndUnlitGeneric_PS },
	{ "vertexlit_and_unlit_generic_bump_vs30", ShaderBlob::VertexLitAndUnlitGeneric_VS },
	{ "vertexlit_and_unlit_generic_bump_ps30", ShaderBlob::VertexLitAndUnlitGeneric_PS },
};

auto VulkanShaderManager::FindOrCreateShader(const CUtlSymbolDbg& id) -> const IShader&
{
	std::lock_guard lock(m_Mutex);

	if (auto found = m_Shaders.find(id); found != m_Shaders.end())
		return found->second;

	// Couldn't find an existing one, we need to create it here
	return m_Shaders.emplace(id, id).first->second;
}

static VariableType ConvertVariableType(const spirv_cross::SPIRType::BaseType& type)
{
#pragma push_macro("CASE")
#undef CASE
#define CASE(typeName) case spirv_cross::SPIRType::BaseType:: ## typeName : return VariableType:: ## typeName

	switch (type)
	{
	default:
		throw VulkanException("Unknown SPIRType", EXCEPTION_DATA());

		CASE(Void);
		CASE(Boolean);
		CASE(SByte);
		CASE(UByte);
		CASE(Short);
		CASE(Int);
		CASE(UInt);
		CASE(Int64);
		CASE(UInt64);
		CASE(Half);
		CASE(Float);
		CASE(Double);
		CASE(Struct);
		CASE(Image);
		CASE(SampledImage);
		CASE(Sampler);
	}

#pragma pop_macro("CASE")
}

ShaderVariable::ShaderVariable(const spirv_cross::Compiler& comp, uint32_t id) :
	m_Name(comp.get_name(id))
{
	const auto& type = comp.expression_type(id);
	//assert(!type.pointer);
	m_Type = ConvertVariableType(type.basetype);
}

ShaderVariable::ShaderVariable(std::string&& name) :
	m_Name(std::move(name))
{
}

SpecializationConstant::SpecializationConstant(const spirv_cross::Compiler& comp, uint32_t id, uint32_t constID) :
	ShaderVariable(comp, id),
	m_ConstantID(constID)
{
}

VertexAttribute::VertexAttribute(const spirv_cross::Compiler& comp, uint32_t id) :
	ShaderVariable(comp, id),
	m_Semantic(comp.get_decoration_string(id, spv::Decoration::DecorationHlslSemanticGOOGLE)),
	m_Location(comp.get_decoration(id, spv::Decoration::DecorationLocation))
{
}

StructMember::StructMember(const spirv_cross::Compiler& comp, uint32_t parent, uint32_t index) :
	ShaderVariable(std::string(comp.get_member_name(parent, index))),
	m_Parent(parent)
{
	const auto& parentType = comp.get_type(parent);
	m_Offset = comp.type_struct_member_offset(parentType, index);
}

Struct::Struct(const spirv_cross::Compiler& comp, const spirv_cross::Resource& resource) :
	m_Name(comp.get_name(resource.base_type_id))
{
	const auto& type = comp.get_type(resource.base_type_id);

	m_Size = comp.get_declared_struct_size(type);

	for (size_t i = 0; i < type.member_types.size(); i++)
		m_Members.emplace_back(comp, resource.base_type_id, i);
}

ShaderResource::ShaderResource(const spirv_cross::Compiler& comp, uint32_t id) :
	m_Binding(comp.get_decoration(id, spv::Decoration::DecorationBinding))
{
}

UniformBuffer::UniformBuffer(const spirv_cross::Compiler& comp, const spirv_cross::Resource& resource) :
	Struct(comp, resource),
	ShaderResource(comp, resource.id)
{
}

Sampler::Sampler(const spirv_cross::Compiler& comp, uint32_t id) :
	ShaderVariable(comp, id),
	ShaderResource(comp, id)
{
}

Texture::Texture(const spirv_cross::Compiler& comp, uint32_t id) :
	ShaderVariable(comp, id),
	ShaderResource(comp, id)
{
}

static ReflectionData CreateReflectionData(const void* data, size_t byteSize)
{
	ReflectionData retVal;

	const spirv_cross::Compiler comp(reinterpret_cast<const uint32_t*>(data), byteSize / sizeof(uint32_t));

	switch (comp.get_execution_model())
	{
	case spv::ExecutionModel::ExecutionModelFragment:
		retVal.m_ShaderStage = vk::ShaderStageFlagBits::eFragment;
		break;
	case spv::ExecutionModel::ExecutionModelVertex:
		retVal.m_ShaderStage = vk::ShaderStageFlagBits::eVertex;
		break;

	default:
		throw VulkanException("Unknown shader type", EXCEPTION_DATA());
	}

	// Specialization constants
	for (const auto& specConstIn : comp.get_specialization_constants())
		retVal.m_SpecConstants.emplace_back(comp, specConstIn.id, specConstIn.constant_id);

	const auto resources = comp.get_shader_resources();
	for (const auto& input : resources.stage_inputs)
		retVal.m_VertexInputs.emplace_back(comp, input.id);

	for (const auto& output : resources.stage_outputs)
		retVal.m_VertexOutputs.emplace_back(comp, output.id);

	for (const auto& uniformBuf : resources.uniform_buffers)
		retVal.m_UniformBuffers.emplace_back(comp, uniformBuf);

	for (const auto& sampler : resources.separate_samplers)
		retVal.m_Samplers.emplace_back(comp, sampler.id);

	for (const auto& texture : resources.separate_images)
		retVal.m_Textures.emplace_back(comp, texture.id);

	return retVal;
}

VulkanShaderManager::CompiledShader::CompiledShader(const CUtlSymbolDbg& name) :
	m_Name(name)
{
	const ShaderInfo& shaderInfo = s_ShaderBlobMapping.at(m_Name.String());
	m_Info = &shaderInfo;

	vk::ShaderModuleCreateInfo ci;

	const void* blobData;
	if (!TF2Vulkan::GetShaderBlob(shaderInfo.m_Blob, blobData, ci.codeSize))
		throw VulkanException("Failed to get shader blob", EXCEPTION_DATA());

	ci.pCode = reinterpret_cast<const uint32_t*>(blobData);

	m_ReflectionData = CreateReflectionData(blobData, ci.codeSize);

	m_Shader = g_ShaderDevice.GetVulkanDevice().createShaderModuleUnique(ci);
	g_ShaderDevice.SetDebugName(m_Shader, m_Name.String());
}

void VulkanShaderManager::CompiledShader::CreateSpecializationInfo(uint32_t combo,
	vk::SpecializationInfo& info, std::vector<vk::SpecializationMapEntry>& entries,
	std::vector<std::byte>& data) const
{
	const auto& reflData = GetReflectionData();

	// TODO: Don't set spec constants to their default values
	for (size_t i = 0; i < m_Info->m_SpecConstMappingsCount; i++)
	{
		const auto& mapping = m_Info->m_SpecConstMappings[i];

		auto value = (combo >> mapping.m_ComboOffset) & mapping.m_ComboMask;

		auto foundConst = std::find_if(reflData.m_SpecConstants.begin(), reflData.m_SpecConstants.end(),
			[&](const SpecializationConstant & c) { return c.m_Name == mapping.m_SpecConstName; });
		if (foundConst == reflData.m_SpecConstants.end())
			continue;

		auto& entry = entries.emplace_back();
		entry.constantID = foundConst->m_ConstantID;
		entry.size = sizeof(uint32_t);
		Util::SafeConvert(data.size(), entry.offset);
		Util::Buffer::Put(data, value);
	}

	info.pMapEntries = entries.data();
	info.mapEntryCount = entries.size();
	Util::SafeConvert(data.size(), info.dataSize);
	info.pData = data.data();
}
