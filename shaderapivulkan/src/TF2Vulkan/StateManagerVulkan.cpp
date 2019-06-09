#include "interface/internal/IShaderAPIInternal.h"
#include "IShaderAPI/IShaderAPI_TextureManager.h"
#include "IStateManagerVulkan.h"
#include "LogicalState.h"
#include "MaterialSystemHardwareConfig.h"
#include "TF2Vulkan/SamplerSettings.h"
#include "interface/internal/IBufferPoolInternal.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IShaderInternal.h"
#include "shaders/VulkanShaderManager.h"
#include "VulkanFactories.h"
#include "TF2Vulkan/FormatInfo.h"

#include <TF2Vulkan/Util/AutoInit.h>
#include <TF2Vulkan/Util/Color.h>
#include <TF2Vulkan/Util/MemoryPool.h>
#include <TF2Vulkan/Util/shaderapi_ishaderdynamic.h>
#include <TF2Vulkan/Util/StackArray.h>
#include <TF2Vulkan/Util/std_algorithm.h>
#include <TF2Vulkan/Util/std_array.h>

#include <stdshader_vulkan/ShaderData.h>

#include <materialsystem/imesh.h>

#undef min
#undef max

#include <forward_list>
#include <mutex>
#include <unordered_map>

using namespace TF2Vulkan;

static constexpr auto BINDING_SAMPLER_OFFSET = 100;
static constexpr auto BINDING_TEXTURE_OFFSET = 200;

namespace
{
	struct SamplerKey
	{
		SamplerKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState, Sampler_t sampler);
		SamplerKey(const SamplerSettings& settings) : m_Settings(settings) {}
		DEFAULT_STRONG_EQUALITY_OPERATOR(SamplerKey);

		SamplerSettings m_Settings;
	};
}

STD_HASH_DEFINITION(SamplerKey,
	v.m_Settings
);

namespace
{
	struct RenderPassKey
	{
		constexpr RenderPassKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_STRONG_ORDERING_OPERATOR(RenderPassKey);

		ShaderAPITextureHandle_t m_OMDepthRT;
		std::array<ShaderAPITextureHandle_t, 4> m_OMColorRTs;
	};
}

STD_HASH_DEFINITION(RenderPassKey,
	v.m_OMDepthRT,
	v.m_OMColorRTs
);

namespace
{
	struct DescriptorSetLayoutKey
	{
		DescriptorSetLayoutKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_STRONG_ORDERING_OPERATOR(DescriptorSetLayoutKey);

		uint32_t m_Texture2DCount = 0;
		uint32_t m_SamplerCount = 0;
		Util::InPlaceVector<uint32_t, 8> m_UniformBuffers;
	};
}

STD_HASH_DEFINITION(DescriptorSetLayoutKey,
	v.m_Texture2DCount,
	v.m_SamplerCount,
	v.m_UniformBuffers
);

namespace
{
	struct PipelineLayoutKey : DescriptorSetLayoutKey
	{
		PipelineLayoutKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState,
			const IMesh& mesh);
		DEFAULT_STRONG_ORDERING_OPERATOR(PipelineLayoutKey);

		// TODO: Pipeline layout only needs to know shader groups, not specific instances
		const IShaderInstanceInternal* m_VSShaderInstance;
		VertexFormat m_VSVertexFormat;

		const IShaderInstanceInternal* m_PSShaderInstance;
	};
}

STD_HASH_DEFINITION(PipelineLayoutKey,
	v.m_VSShaderInstance,
	v.m_VSVertexFormat,

	v.m_PSShaderInstance
);

namespace
{
	struct PipelineKey final : RenderPassKey, PipelineLayoutKey
	{
		PipelineKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState,
			const IMesh& mesh);
		DEFAULT_STRONG_ORDERING_OPERATOR(PipelineKey);

		ShaderDepthFunc_t m_DepthCompareFunc;
		bool m_DepthTest;
		bool m_DepthWrite;

		bool m_RSBackFaceCulling;
		ShaderPolyMode_t m_RSPolyMode;

		bool m_OMAlphaBlending;
		ShaderBlendFactor_t m_OMSrcFactor;
		ShaderBlendFactor_t m_OMDstFactor;

		Util::InPlaceVector<ShaderViewport_t, 4> m_Viewports;
	};
}

STD_HASH_DEFINITION(PipelineKey,
	static_cast<const RenderPassKey&>(v),
	static_cast<const PipelineLayoutKey&>(v),

	v.m_DepthCompareFunc,
	v.m_DepthTest,
	v.m_DepthWrite,

	v.m_RSBackFaceCulling,
	v.m_RSPolyMode,

	v.m_OMAlphaBlending,
	v.m_OMSrcFactor,
	v.m_OMDstFactor,

	v.m_Viewports
);

namespace
{
	struct RenderPass;
	struct FramebufferKey final
	{
		FramebufferKey(const RenderPass& rp);
		DEFAULT_STRONG_ORDERING_OPERATOR(FramebufferKey);

		struct RTRef
		{
			RTRef(IShaderAPITexture* tex);
			DEFAULT_STRONG_ORDERING_OPERATOR(RTRef);
			vk::ImageView m_ImageView;
			vk::Extent2D m_Extent;
			bool operator!() const { return !m_ImageView; }
		};

		std::array<RTRef, 4> m_OMColorRTs;
		RTRef m_OMDepthRT;

		const RenderPass* m_RenderPass;
	};
}

STD_HASH_DEFINITION(FramebufferKey::RTRef,
	v.m_ImageView,
	v.m_Extent
);

STD_HASH_DEFINITION(FramebufferKey,
	v.m_OMDepthRT,
	v.m_OMColorRTs,
	v.m_RenderPass
);

namespace
{
	struct DescriptorSetLayout;
	struct DescriptorPoolKey final
	{
		DescriptorPoolKey(const DescriptorSetLayout& layout) : m_Layout(&layout) {}
		DEFAULT_STRONG_ORDERING_OPERATOR(DescriptorPoolKey);

		const DescriptorSetLayout* m_Layout;
	};
}

STD_HASH_DEFINITION(DescriptorPoolKey,
	v.m_Layout
);

namespace
{
	struct DescriptorSetKey final
	{
		DescriptorSetKey(const DescriptorSetLayout& layout, const LogicalDynamicState& dynamicState);

		DEFAULT_STRONG_EQUALITY_OPERATOR(DescriptorSetKey);

		struct UBRef
		{
			UBRef(const BufferPoolEntry& buf);
			vk::Buffer m_Buffer;
			size_t m_Length = 0;

			DEFAULT_STRONG_EQUALITY_OPERATOR(UBRef);

			operator bool() const { return !!m_Buffer; }
		};

		const DescriptorSetLayout* m_Layout = nullptr;
		std::array<UBRef, 8> m_UniformBuffers;
		Util::InPlaceVector<ShaderAPITextureHandle_t, MAX_SHADER_RESOURCE_BINDINGS> m_BoundTextures = {};
		Util::InPlaceVector<SamplerSettings, MAX_SHADER_RESOURCE_BINDINGS> m_BoundSamplers = {};
	};
}

STD_HASH_DEFINITION(DescriptorSetKey::UBRef,
	v.m_Buffer,
	v.m_Length
);

STD_HASH_DEFINITION(DescriptorSetKey,
	v.m_Layout,
	v.m_UniformBuffers,
	v.m_BoundTextures,
	v.m_BoundSamplers
);

namespace
{
	struct DescriptorSet final
	{
		DescriptorSet(const DescriptorSetKey& key);

		vk::UniqueDescriptorSet m_DescriptorSet;

		std::vector<vk::WriteDescriptorSet> m_Writes;
		std::list<vk::DescriptorBufferInfo> m_BufferInfos;
		std::vector<vk::DescriptorImageInfo> m_ImageInfos;
		std::vector<vk::DescriptorImageInfo> m_SamplerInfos;
	};

	struct Sampler final
	{
		Sampler(const SamplerKey& key);

		vk::SamplerCreateInfo m_CreateInfo;
		vk::UniqueSampler m_Sampler;

		void FixupPointers();
		bool operator!() const { return !m_Sampler; }
	};

	struct DescriptorPool final
	{
		DescriptorPool(const DescriptorPoolKey& key);

		std::vector<vk::DescriptorPoolSize> m_Sizes;

		vk::DescriptorPoolCreateInfo m_CreateInfo;
		vk::UniqueDescriptorPool m_DescriptorPool;

		void FixupPointers();
		bool operator!() const { return !m_DescriptorPool; }
	};

	struct DescriptorSetLayout final
	{
		DescriptorSetLayout(const DescriptorSetLayoutKey& key);

		std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;
		vk::DescriptorSetLayoutCreateInfo m_CreateInfo;
		vk::UniqueDescriptorSetLayout m_Layout;

		void FixupPointers();
		bool operator!() const { return !m_Layout; }
	};

	struct PipelineLayout final
	{
		PipelineLayout(const PipelineLayoutKey& key);

		std::vector<const DescriptorSetLayout*> m_SetLayouts;
		std::vector<vk::PushConstantRange> m_PushConstantRanges;

		vk::PipelineLayoutCreateInfo m_CreateInfo;
		vk::UniquePipelineLayout m_Layout;

		void FixupPointers();
		bool operator!() const { return !m_Layout; }
	};

	struct Subpass final
	{
		std::vector<vk::AttachmentReference> m_InputAttachments;
		std::vector<vk::AttachmentReference> m_ColorAttachments;
		vk::AttachmentReference m_DepthStencilAttachment;

		vk::SubpassDescription m_CreateInfo;

		void FixupPointers();
	};

	struct RenderPass final
	{
		RenderPass(const RenderPassKey& key);

		std::vector<vk::AttachmentDescription> m_Attachments;
		std::vector<vk::SubpassDependency> m_Dependencies;
		std::vector<Subpass> m_Subpasses;

		vk::RenderPassCreateInfo m_CreateInfo;
		vk::UniqueRenderPass m_RenderPass;

		RenderPassKey m_Key;

		void FixupPointers();
		bool operator!() const { return !m_RenderPass; }
	};

	struct Framebuffer final
	{
		Framebuffer(const FramebufferKey& key);

		std::vector<vk::ImageView> m_Attachments;

		vk::FramebufferCreateInfo m_CreateInfo;
		vk::UniqueFramebuffer m_Framebuffer;

		void FixupPointers();
		bool operator!() const { return !m_Framebuffer; }
	};

	struct ShaderStageCreateInfo
	{
		ShaderStageCreateInfo(const IShaderInstanceInternal& shader, vk::ShaderStageFlagBits type);

		const IVulkanShader* m_Shader = nullptr;

		vk::SpecializationInfo m_SpecializationInfo;
		vk::PipelineShaderStageCreateInfo m_CreateInfo;

		void FixupPointers();
	};

	struct Pipeline final
	{
		Pipeline(const PipelineKey& key, const PipelineLayout& layout, const RenderPass& renderPass);

		std::vector<ShaderStageCreateInfo> m_ShaderStageCIs;

		std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
		std::vector<vk::VertexInputBindingDescription> m_VertexInputBindingDescriptions;
		vk::PipelineVertexInputStateCreateInfo m_VertexInputStateCI;

		vk::PipelineInputAssemblyStateCreateInfo m_InputAssemblyStateCI;

		std::vector<vk::Viewport> m_Viewports;
		vk::Rect2D m_Scissor;
		vk::PipelineViewportStateCreateInfo m_ViewportStateCI;

		vk::PipelineRasterizationStateCreateInfo m_RasterizationStateCI;

		std::vector<vk::PipelineColorBlendAttachmentState> m_ColorBlendAttachmentStates;
		vk::PipelineColorBlendStateCreateInfo m_ColorBlendStateCI;

		vk::PipelineMultisampleStateCreateInfo m_MultisampleStateCI;

		vk::PipelineDepthStencilStateCreateInfo m_DepthStencilStateCI;

		const PipelineLayout* m_Layout = nullptr;
		const RenderPass* m_RenderPass = nullptr;

		vk::GraphicsPipelineCreateInfo m_CreateInfo;
		vk::UniquePipeline m_Pipeline;

		void FixupPointers();
		bool operator!() const { return !m_Pipeline; }
		VulkanStateID m_ID;
	};
}

namespace TF2Vulkan
{
	class StateManagerVulkan final : public IStateManagerVulkan
	{
	public:
		void ApplyState(VulkanStateID stateID,
			const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState,
			const IMesh& mesh, IVulkanCommandBuffer& buf) override;

		VulkanStateID FindOrCreateState(const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState, const IMesh& mesh) override;

		const DescriptorSet& FindOrCreateDescriptorSet(const DescriptorSetKey& key);
		const DescriptorPool& FindOrCreateDescriptorPool(const DescriptorPoolKey& key);
		const Sampler& FindOrCreateSampler(const SamplerKey& sampler);
		const DescriptorSetLayout& FindOrCreateDescriptorSetLayout(const DescriptorSetLayoutKey& key);

		const vk::Buffer& GetDummyUniformBuffer() const { return g_ShaderDevice.GetDummyUniformBuffer(); }

	private:
		void ApplyRenderPass(const RenderPass& renderPass, IVulkanCommandBuffer& buf);
		void ApplyDescriptorSets(const Pipeline& pipeline,
			const LogicalDynamicState& dynamicState, IVulkanCommandBuffer& buf);

		const PipelineLayout& FindOrCreatePipelineLayout(const PipelineLayoutKey& key);
		const RenderPass& FindOrCreateRenderPass(const RenderPassKey& key);
		const Framebuffer& FindOrCreateFramebuffer(const FramebufferKey& key);

		std::recursive_mutex m_Mutex;

		std::unordered_map<PipelineKey, Pipeline> m_StatesToPipelines;
		std::vector<const Pipeline*> m_IDsToPipelines;
		std::unordered_map<PipelineLayoutKey, PipelineLayout> m_StatesToLayouts;
		std::unordered_map<RenderPassKey, RenderPass> m_StatesToRenderPasses;
		std::unordered_map<FramebufferKey, Framebuffer> m_StatesToFramebuffers;
		std::unordered_map<DescriptorPoolKey, DescriptorPool> m_StatesToDescPools;
		std::unordered_map<DescriptorSetKey, DescriptorSet> m_StatesToDescSets;
		std::unordered_map<DescriptorSetLayoutKey, DescriptorSetLayout> m_StatesToDescSetLayouts;
		std::unordered_map<SamplerKey, Sampler> m_StatesToSamplers;

		VulkanStateID m_ActiveState = VulkanStateID::Invalid;
	};
}

static StateManagerVulkan s_SMVulkan;
IStateManagerVulkan& TF2Vulkan::g_StateManagerVulkan = s_SMVulkan;

template<typename T, typename TSize>
static void AttachVector(const T*& destData, TSize& destSize, const std::vector<T>& src)
{
	destData = src.data();
	Util::SafeConvert(src.size(), destSize);
}

const DescriptorSetLayout& StateManagerVulkan::FindOrCreateDescriptorSetLayout(const DescriptorSetLayoutKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToDescSetLayouts.try_emplace(key, key).first->second;
}

ShaderStageCreateInfo::ShaderStageCreateInfo(
	const IShaderInstanceInternal& shader, vk::ShaderStageFlagBits type) :
	m_Shader(&shader.GetGroup().GetVulkanShader())
{
	shader.GetSpecializationInfo(m_SpecializationInfo);

	auto& ci = m_CreateInfo;
	ci.stage = type;
	ci.module = m_Shader->GetModule();
	ci.pName = "main"; // Shader entry point
	ci.pSpecializationInfo = &m_SpecializationInfo;
}

void ShaderStageCreateInfo::FixupPointers()
{
	m_CreateInfo.pSpecializationInfo = &m_SpecializationInfo;
}

DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutKey& key)
{
	const auto CreateBinding = [](vk::DescriptorType type, uint32_t count, uint32_t binding)
	{
		vk::DescriptorSetLayoutBinding newEntry;
		newEntry.binding = binding;
		newEntry.descriptorType = type;
		newEntry.descriptorCount = count;
		newEntry.stageFlags = vk::ShaderStageFlagBits::eAllGraphics;
		return newEntry;
	};

	// Bindings
	//if (key.m_Texture2DCount > 0)
		m_Bindings.emplace_back(CreateBinding(vk::DescriptorType::eSampledImage,
			Util::algorithm::max(key.m_Texture2DCount, 1), TF2Vulkan::Shaders::BINDING_TEX2D));

	//if (key.m_SamplerCount > 0)
		m_Bindings.emplace_back(CreateBinding(vk::DescriptorType::eSampler,
			Util::algorithm::max(key.m_SamplerCount, 1), TF2Vulkan::Shaders::BINDING_SAMPLERS));

	for (const auto bufIndex : key.m_UniformBuffers)
		m_Bindings.emplace_back(CreateBinding(vk::DescriptorType::eUniformBufferDynamic, 1, bufIndex));

	// Descriptor set layout
	{
		auto& ci = m_CreateInfo;
		AttachVector(ci.pBindings, ci.bindingCount, m_Bindings);

		auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
		m_Layout = device.createDescriptorSetLayoutUnique(ci, g_ShaderDeviceMgr.GetAllocationCallbacks());
		g_ShaderDevice.SetDebugName(m_Layout, "TF2Vulkan Descriptor Set Layout 0x%zX (%u t2d, %u s)",
			Util::hash_value(key), key.m_Texture2DCount, key.m_SamplerCount);
	}
}

PipelineLayout::PipelineLayout(const PipelineLayoutKey& key)
{
	// Descriptor set layouts
	m_SetLayouts.push_back(&s_SMVulkan.FindOrCreateDescriptorSetLayout(key));

	// Pipeline Layout
	{
		auto& ci = m_CreateInfo;

		std::vector<vk::DescriptorSetLayout> setLayouts;
		for (auto& sl : m_SetLayouts)
			setLayouts.push_back(sl->m_Layout.get());

		AttachVector(ci.pSetLayouts, ci.setLayoutCount, setLayouts);
		AttachVector(ci.pPushConstantRanges, ci.pushConstantRangeCount, m_PushConstantRanges);

		auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
		m_Layout = device.createPipelineLayoutUnique(
			ci, g_ShaderDeviceMgr.GetAllocationCallbacks());

		g_ShaderDevice.SetDebugName(m_Layout, "TF2Vulkan Pipeline Layout 0x%zX", Util::hash_value(key));
	}
}

static constexpr IShaderAPITexture* TryFindTexture(ShaderAPITextureHandle_t handle, bool depth = false)
{
	if (handle < 0)
		return nullptr;

	if (handle == 0)
	{
		if (!depth)
			return &g_ShaderDevice.GetBackBufferColorTexture();
		else
			return &g_ShaderDevice.GetBackBufferDepthTexture();
	}

	return &g_TextureManager.GetTexture(handle);
}

static IShaderAPITexture& FindTexture(ShaderAPITextureHandle_t handle, bool depth = false)
{
	auto found = TryFindTexture(handle, depth);
	if (!found)
		throw VulkanException("TryFindTexture returned nullptr", EXCEPTION_DATA());

	return *found;
}

RenderPass::RenderPass(const RenderPassKey& key) :
	m_Key(key)
{
	// Subpass 0
	{
		Subpass& sp = m_Subpasses.emplace_back();

		// Color attachments
		{
			for (auto& colorTexID : key.m_OMColorRTs)
			{
				if (colorTexID < 0)
					continue;

				auto& colorTex = FindTexture(colorTexID);

				vk::AttachmentDescription& att = m_Attachments.emplace_back();
				att.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
				att.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
				att.samples = vk::SampleCountFlagBits::e1;
				att.loadOp = vk::AttachmentLoadOp::eLoad; // TODO: Switch to eClear when we call ClearBuffers()
				att.storeOp = vk::AttachmentStoreOp::eStore;
				att.stencilLoadOp = vk::AttachmentLoadOp::eLoad; // TODO: Switch to eClear when we call ClearBuffers()
				att.stencilStoreOp = vk::AttachmentStoreOp::eStore;

				att.format = colorTex.GetImageCreateInfo().format;

				vk::AttachmentReference& attRef = sp.m_ColorAttachments.emplace_back();
				attRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
				Util::SafeConvert(m_Attachments.size() - 1, attRef.attachment);
			}
		}

		// Depth attachments
		if (key.m_OMDepthRT >= 0)
		{
			vk::AttachmentDescription& att = m_Attachments.emplace_back();

			att.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			att.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			att.samples = vk::SampleCountFlagBits::e1;
			att.loadOp = vk::AttachmentLoadOp::eClear;//eLoad; // TODO: Switch to eClear when we call ClearBuffers()
			att.storeOp = vk::AttachmentStoreOp::eStore;
			att.stencilLoadOp = vk::AttachmentLoadOp::eLoad; // TODO: Switch to eClear when we call ClearBuffers()
			att.stencilStoreOp = vk::AttachmentStoreOp::eStore;

			att.format = FindTexture(key.m_OMDepthRT, true).GetImageCreateInfo().format;

			vk::AttachmentReference& attRef = sp.m_DepthStencilAttachment;
			attRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			Util::SafeConvert(m_Attachments.size() - 1, attRef.attachment);

			sp.m_CreateInfo.pDepthStencilAttachment = &sp.m_DepthStencilAttachment;
		}

		AttachVector(sp.m_CreateInfo.pInputAttachments, sp.m_CreateInfo.inputAttachmentCount, sp.m_InputAttachments);
		AttachVector(sp.m_CreateInfo.pColorAttachments, sp.m_CreateInfo.colorAttachmentCount, sp.m_ColorAttachments);
	}

	// Render pass
	{
		auto& ci = m_CreateInfo;

		AttachVector(ci.pAttachments, ci.attachmentCount, m_Attachments);
		AttachVector(ci.pDependencies, ci.dependencyCount, m_Dependencies);

		std::vector<vk::SubpassDescription> subpassTemp;
		for (auto& sp : m_Subpasses)
			subpassTemp.push_back(sp.m_CreateInfo);

		AttachVector(ci.pSubpasses, ci.subpassCount, subpassTemp);

		auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
		m_RenderPass = device.createRenderPassUnique(
			ci, g_ShaderDeviceMgr.GetAllocationCallbacks());
		g_ShaderDevice.SetDebugName(m_RenderPass, "TF2Vulkan Render Pass 0x%zX", Util::hash_value(key));
	}
}

static vk::BlendFactor ConvertBlendFactor(ShaderBlendFactor_t blendFactor)
{
	switch (blendFactor)
	{
	case SHADER_BLEND_ZERO:                 return vk::BlendFactor::eZero;
	case SHADER_BLEND_ONE:                  return vk::BlendFactor::eOne;
	case SHADER_BLEND_DST_COLOR:            return vk::BlendFactor::eDstColor;
	case SHADER_BLEND_ONE_MINUS_DST_COLOR:  return vk::BlendFactor::eOneMinusDstColor;
	case SHADER_BLEND_SRC_ALPHA:            return vk::BlendFactor::eSrcAlpha;
	case SHADER_BLEND_ONE_MINUS_SRC_ALPHA:  return vk::BlendFactor::eOneMinusSrcAlpha;
	case SHADER_BLEND_DST_ALPHA:            return vk::BlendFactor::eDstAlpha;
	case SHADER_BLEND_ONE_MINUS_DST_ALPHA:  return vk::BlendFactor::eOneMinusDstAlpha;
	case SHADER_BLEND_SRC_ALPHA_SATURATE:   return vk::BlendFactor::eSrcAlphaSaturate;
	case SHADER_BLEND_SRC_COLOR:            return vk::BlendFactor::eSrcColor;
	case SHADER_BLEND_ONE_MINUS_SRC_COLOR:  return vk::BlendFactor::eOneMinusSrcColor;

	default:
		throw VulkanException("Unknown ShaderBlendFactor_t", EXCEPTION_DATA());
	}
}

static vk::CompareOp ConvertCompareOp(ShaderDepthFunc_t op)
{
	switch (op)
	{
	case SHADER_DEPTHFUNC_NEVER:           return vk::CompareOp::eNever;
	case SHADER_DEPTHFUNC_NEARER:          return vk::CompareOp::eLess;
	case SHADER_DEPTHFUNC_EQUAL:           return vk::CompareOp::eEqual;
	case SHADER_DEPTHFUNC_NEAREROREQUAL:   return vk::CompareOp::eLessOrEqual;
	case SHADER_DEPTHFUNC_FARTHER:         return vk::CompareOp::eGreater;
	case SHADER_DEPTHFUNC_NOTEQUAL:        return vk::CompareOp::eNotEqual;
	case SHADER_DEPTHFUNC_FARTHEROREQUAL:  return vk::CompareOp::eGreaterOrEqual;
	case SHADER_DEPTHFUNC_ALWAYS:          return vk::CompareOp::eAlways;

	default:
		throw VulkanException("Unknown ShaderDepthFunc_t", EXCEPTION_DATA());
	}
}

static vk::Format ConvertVertexFormat(const ShaderReflection::VertexAttribute& var)
{
	DataFormat componentType;
	uint_fast8_t componentSize;

	switch (var.m_Type)
	{
	default:
		assert(!"Unknown/unexpected vertex attribute type");
	case ShaderReflection::VariableType::Float:
		componentType = DataFormat::SFloat;
		componentSize = 4;
		break;

	case ShaderReflection::VariableType::Int:
		componentType = DataFormat::SInt;
		componentSize = 4;
		break;

	case ShaderReflection::VariableType::UInt:
	case ShaderReflection::VariableType::Boolean:
		componentType = DataFormat::UInt;
		componentSize = 4;
		break;
	}

	return FormatInfo::ConvertDataFormat(componentType, var.m_ComponentCount, componentSize);
}

Pipeline::Pipeline(const PipelineKey& key, const PipelineLayout& layout,
	const RenderPass& renderPass) :
	m_Layout(&layout),
	m_RenderPass(&renderPass)
{
	// Shader stage create info(s)
	{
		auto& cis = m_ShaderStageCIs;
		cis.emplace_back(*key.m_VSShaderInstance, vk::ShaderStageFlagBits::eVertex);
		cis.emplace_back(*key.m_PSShaderInstance, vk::ShaderStageFlagBits::eFragment);
	}

	// Vertex input state create info
	{
		auto& attrs = m_VertexInputAttributeDescriptions;

		const auto& vertexShaderRefl = key.m_VSShaderInstance->GetGroup().GetVulkanShader().GetReflectionData();

		VertexFormat::Element vertexElements[VERTEX_ELEMENT_NUMELEMENTS];
		size_t totalVertexSize;
		const auto vertexElementCount = key.m_VSVertexFormat.GetVertexElements(vertexElements, std::size(vertexElements), &totalVertexSize);
		using VIAD = vk::VertexInputAttributeDescription;
		for (uint_fast8_t i = 0; i < vertexElementCount; i++)
		{
			const auto& vertexElement = vertexElements[i];

			VIAD attr;
			attr.offset = vertexElement.m_Offset;
			attr.format = vertexElement.m_Type->GetVKFormat();
			attr.binding = 0;

			static constexpr auto INVALID_LOCATION = std::numeric_limits<uint32_t>::max();
			attr.location = INVALID_LOCATION;
			for (const auto& input : vertexShaderRefl.m_VertexInputs)
			{
				if (input.m_Semantic == vertexElement.m_Type->m_Semantic)
				{
					attr.location = input.m_Location;
					break;
				}
			}

			if (attr.location == INVALID_LOCATION)
			{
				assert(!"Failed to find matching semantic");
				continue;
			}

			attrs.emplace_back(attr);
		}

		for (const auto& input : vertexShaderRefl.m_VertexInputs)
		{
			bool found = false;
			for (auto& attr : attrs)
			{
				if (attr.location == input.m_Location)
				{
					found = true;
					break;
				}
			}

			if (found)
				continue;

			// Otherwise, insert an empty one
			//attrs.emplace_back(VIAD(input.m_Location, 1, vk::Format::eR32G32B32A32Sfloat));
			attrs.emplace_back(VIAD(input.m_Location, 1, ConvertVertexFormat(input)));
		}

		auto & binds = m_VertexInputBindingDescriptions;
		binds.emplace_back(vk::VertexInputBindingDescription(0, totalVertexSize));

		// "Fake" binding with no real data
		binds.emplace_back(vk::VertexInputBindingDescription(1, sizeof(float) * 4, vk::VertexInputRate::eInstance));

		auto& ci = m_VertexInputStateCI;
		AttachVector(ci.pVertexAttributeDescriptions, ci.vertexAttributeDescriptionCount, attrs);
		AttachVector(ci.pVertexBindingDescriptions, ci.vertexBindingDescriptionCount, binds);
	}

	// Vertex input assembly state create info
	{
		auto& ci = m_InputAssemblyStateCI;
		ci.topology = vk::PrimitiveTopology::eTriangleList;
	}

	// Viewport/scissor state
	{
		// Viewport(s)
		{
			auto& ci = m_ViewportStateCI;
			for (const auto& vpIn : key.m_Viewports)
			{
				auto& vpOut = m_Viewports.emplace_back();
				Util::SafeConvert(vpIn.m_nWidth, vpOut.width);
				Util::SafeConvert(vpIn.m_nHeight, vpOut.height);
				Util::SafeConvert(vpIn.m_nTopLeftX, vpOut.x);
				Util::SafeConvert(vpIn.m_nTopLeftY, vpOut.y);
				Util::SafeConvert(vpIn.m_flMinZ, vpOut.minDepth);
				Util::SafeConvert(vpIn.m_flMaxZ, vpOut.maxDepth);
			}

			AttachVector(ci.pViewports, ci.viewportCount, m_Viewports);
		}

		// Scissor(s)
		{
			const auto& vp = m_ViewportStateCI.pViewports[0];
			m_Scissor.offset.x = vp.x;
			m_Scissor.offset.y = vp.y;
			m_Scissor.extent.width = vp.width;
			m_Scissor.extent.height = vp.height;
			m_ViewportStateCI.pScissors = &m_Scissor;
			m_ViewportStateCI.scissorCount = 1;
		}
	}

	// Rasterization state create info
	{
		auto& ci = m_RasterizationStateCI;

		ci.frontFace = vk::FrontFace::eClockwise; // Reversed, because we have to invert Y in our vertex shader
		ci.lineWidth = 1; // default
		ci.cullMode = key.m_RSBackFaceCulling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone; // FIXME: Temp disabled
		switch (key.m_RSPolyMode)
		{
		default:
			assert(!"Unknown polygon mode");
		case SHADER_POLYMODE_FILL:
			ci.polygonMode = vk::PolygonMode::eFill;
			break;
		case SHADER_POLYMODE_LINE:
			ci.polygonMode = vk::PolygonMode::eLine;
			break;
		case SHADER_POLYMODE_POINT:
			ci.polygonMode = vk::PolygonMode::ePoint;
			break;
		}
	}

	// Multisample state create info
	{
		auto& ci = m_MultisampleStateCI;
		ci.rasterizationSamples = vk::SampleCountFlagBits::e1;
	}

	// Color blend state create info
	{
		auto& ci = m_ColorBlendStateCI;

		ci.setBlendConstants({ 1, 1, 1, 1 });

		auto& att = m_ColorBlendAttachmentStates.emplace_back();

		att.blendEnable = key.m_OMAlphaBlending;
		if (att.blendEnable)
		{
			att.srcAlphaBlendFactor = att.srcColorBlendFactor = ConvertBlendFactor(key.m_OMSrcFactor);
			att.dstAlphaBlendFactor = att.dstColorBlendFactor = ConvertBlendFactor(key.m_OMDstFactor);
		}

		att.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA;

		AttachVector(ci.pAttachments, ci.attachmentCount, m_ColorBlendAttachmentStates);
	}

	// Depth stencil state
	{
		auto& ci = m_DepthStencilStateCI;
		m_CreateInfo.pDepthStencilState = &ci;

		ci.depthTestEnable = key.m_DepthTest;
		ci.depthWriteEnable = key.m_DepthWrite;
		ci.depthCompareOp = ConvertCompareOp(key.m_DepthCompareFunc);
	}

	// Graphics pipeline
	{
		vk::GraphicsPipelineCreateInfo& ci = m_CreateInfo;

		FixupPointers();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for (const auto& stage : m_ShaderStageCIs)
			shaderStages.push_back(stage.m_CreateInfo);

		AttachVector(ci.pStages, ci.stageCount, shaderStages);

		// ci.subpass = 0;
		auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
		m_Pipeline = device.createGraphicsPipelineUnique(
			g_ShaderDevice.GetPipelineCache(), ci, g_ShaderDeviceMgr.GetAllocationCallbacks());

		g_ShaderDevice.SetDebugName(m_Pipeline, "TF2Vulkan Graphics Pipeline 0x%zX", Util::hash_value(key));
	}
}

Framebuffer::Framebuffer(const FramebufferKey& key)
{
	// Attachments
	uint32_t width = 0;// std::numeric_limits<uint32_t>::max();
	uint32_t height = 0;// width;

	std::string debugName = "COL{ ";
	// Color attachments
	{
		bool debugNameFirst = true;

		auto& atts = m_Attachments;
		for (auto& colorRT : key.m_OMColorRTs)
		{
			if (!colorRT)
				continue;

			if (debugNameFirst)
				debugNameFirst = false;
			else
				debugName += '/';

			atts.push_back(colorRT.m_ImageView);

			debugName += '\'';
			debugName += std::to_string((VkImageView)colorRT.m_ImageView);
			debugName += '\'';

			if (width == 0 || colorRT.m_Extent.width < width)
				width = colorRT.m_Extent.width;
			if (height == 0 || colorRT.m_Extent.height < height)
				height = colorRT.m_Extent.height;
		}
		debugName += " }";
	}

	// (Optional) depth attachment
	if (!!key.m_OMDepthRT)
	{
		m_Attachments.push_back(key.m_OMDepthRT.m_ImageView);

		debugName += " DEPTH { ";
		debugName += std::to_string((VkImageView)key.m_OMDepthRT.m_ImageView);
		debugName += " }";
	}

	// Framebuffer
	{
		auto& ci = m_CreateInfo;
		ci.width = width;
		ci.height = height;
		ci.layers = 1;

		AttachVector(ci.pAttachments, ci.attachmentCount, m_Attachments);

		ci.renderPass = key.m_RenderPass->m_RenderPass.get();

		auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
		m_Framebuffer = device.createFramebufferUnique(ci, g_ShaderDeviceMgr.GetAllocationCallbacks());
		assert(m_Framebuffer);
		g_ShaderDevice.SetDebugName(m_Framebuffer, debugName.c_str());
	}
}

const Framebuffer& StateManagerVulkan::FindOrCreateFramebuffer(const FramebufferKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToFramebuffers.try_emplace(key, key).first->second;
}

static vk::SamplerAddressMode ConvertAddressMode(ShaderTexWrapMode_t mode)
{
	switch (mode)
	{
	default:
		assert(!"Invalid ShaderTexWrapMode_t");

	case SHADER_TEXWRAPMODE_CLAMP:  return vk::SamplerAddressMode::eClampToEdge;
	case SHADER_TEXWRAPMODE_REPEAT: return vk::SamplerAddressMode::eRepeat;
	case SHADER_TEXWRAPMODE_BORDER: return vk::SamplerAddressMode::eClampToBorder;
	}
}

static vk::Filter ConvertFilter(ShaderTexFilterMode_t mode)
{
	switch (mode)
	{
	default:
		assert(!"Invalid ShaderTexFilterMode_t");

	case SHADER_TEXFILTERMODE_NEAREST:
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST:
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR:
		return vk::Filter::eNearest;

		// We just have this here so we don't hit the assert.
	case SHADER_TEXFILTERMODE_ANISOTROPIC:

	case SHADER_TEXFILTERMODE_LINEAR:
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_NEAREST:
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR:
		return vk::Filter::eLinear;
	}
}

static vk::SamplerMipmapMode ConvertMipmapMode(ShaderTexFilterMode_t mode)
{
	switch (mode)
	{
	default:
		assert(!"Invalid ShaderTexFilterMode_t");

	case SHADER_TEXFILTERMODE_NEAREST:
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_NEAREST:
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_NEAREST:
		return vk::SamplerMipmapMode::eNearest;

	case SHADER_TEXFILTERMODE_LINEAR:
	case SHADER_TEXFILTERMODE_NEAREST_MIPMAP_LINEAR:
	case SHADER_TEXFILTERMODE_LINEAR_MIPMAP_LINEAR:
	case SHADER_TEXFILTERMODE_ANISOTROPIC:
		return vk::SamplerMipmapMode::eLinear;
	}
}

Sampler::Sampler(const SamplerKey& key)
{
	auto& ci = m_CreateInfo;
	ci.addressModeU = ConvertAddressMode(key.m_Settings.m_WrapS);
	ci.addressModeV = ConvertAddressMode(key.m_Settings.m_WrapT);
	ci.addressModeW = ConvertAddressMode(key.m_Settings.m_WrapU);
	ci.minFilter = ConvertFilter(key.m_Settings.m_MinFilter);
	ci.magFilter = ConvertFilter(key.m_Settings.m_MagFilter);
	ci.anisotropyEnable = (key.m_Settings.m_MinFilter == SHADER_TEXFILTERMODE_ANISOTROPIC ||
		key.m_Settings.m_MagFilter == SHADER_TEXFILTERMODE_ANISOTROPIC);
	if (ci.anisotropyEnable)
		ci.maxAnisotropy = 8;

	if (ConvertMipmapMode(key.m_Settings.m_MinFilter) == vk::SamplerMipmapMode::eLinear ||
		ConvertMipmapMode(key.m_Settings.m_MagFilter) == vk::SamplerMipmapMode::eLinear)
	{
		ci.mipmapMode = vk::SamplerMipmapMode::eLinear;
	}
	else
	{
		ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
	}

	auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
	m_Sampler = device.createSamplerUnique(m_CreateInfo,
		g_ShaderDeviceMgr.GetAllocationCallbacks());
	g_ShaderDevice.SetDebugName(m_Sampler, "TF2Vulkan Sampler 0x%zX", Util::hash_value(key));
}

const Sampler& StateManagerVulkan::FindOrCreateSampler(const SamplerKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToSamplers.try_emplace(key, key).first->second;
}

void StateManagerVulkan::ApplyRenderPass(const RenderPass& renderPass, IVulkanCommandBuffer& buf)
{
	LOG_FUNC();

	vk::RenderPassBeginInfo rpInfo;
	rpInfo.renderPass = renderPass.m_RenderPass.get();

	vk::ClearValue clearVal[2];
	clearVal[0].color.float32[0] = 157 / 255.0f;
	clearVal[0].color.float32[1] =  83 / 255.0f;
	clearVal[0].color.float32[2] =  34 / 255.0f;
	clearVal[0].color.float32[3] = 255 / 255.0f;

	clearVal[1].depthStencil.depth = 1;
	clearVal[1].depthStencil.stencil = 0;

	rpInfo.clearValueCount = std::size(clearVal);
	rpInfo.pClearValues = clearVal;

	// Hack!
	const auto& fb = FindOrCreateFramebuffer(renderPass);
	rpInfo.framebuffer = fb.m_Framebuffer.get();

	rpInfo.renderArea.extent.width = fb.m_CreateInfo.width;
	rpInfo.renderArea.extent.height = fb.m_CreateInfo.height;

	if (!buf.IsRenderPassActive(rpInfo, vk::SubpassContents::eInline))
	{
		constexpr auto applyRenderPassColor = TF2VULKAN_RANDOM_COLOR_FROM_LOCATION();
		buf.InsertDebugLabel(applyRenderPassColor, "ApplyRenderPass()");

		if (buf.GetActiveRenderPass())
			buf.endRenderPass();

		buf.beginRenderPass(rpInfo, vk::SubpassContents::eInline);
	}
}

DescriptorSet::DescriptorSet(const DescriptorSetKey& key)
{
	auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();

	assert(key.m_Layout);
	const auto& layout = *key.m_Layout;
	auto& pool = s_SMVulkan.FindOrCreateDescriptorPool(layout);

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = pool.m_DescriptorPool.get();
	allocInfo.pSetLayouts = &layout.m_Layout.get();
	allocInfo.descriptorSetCount = 1;

	m_DescriptorSet = std::move(device.allocateDescriptorSetsUnique(allocInfo).at(0));

	// Uniform buffers
	{
		vk::WriteDescriptorSet write;
		write.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
		write.descriptorCount = 1;
		write.dstSet = m_DescriptorSet.get();

		const auto RecordUniformBuffer = [&](const DescriptorSetKey::UBRef& ub, uint32_t binding)
		{
			if (!ub)
				return;

			auto& bufInfo = m_BufferInfos.emplace_front();
			bufInfo.buffer = ub.m_Buffer;
			bufInfo.range = ub.m_Length;

			write.dstBinding = binding;
			write.pBufferInfo = &bufInfo;
			m_Writes.push_back(write);
		};

		for (size_t i = 0; i < key.m_UniformBuffers.size(); i++)
			RecordUniformBuffer(key.m_UniformBuffers[i], i);
	}

	// Textures
	{
		vk::WriteDescriptorSet& write = m_Writes.emplace_back();
		write.descriptorType = vk::DescriptorType::eSampledImage;
		write.dstSet = m_DescriptorSet.get();
		write.dstBinding = TF2Vulkan::Shaders::BINDING_TEX2D;
		write.descriptorCount = key.m_BoundTextures.size();
		if (write.descriptorCount > 0)
		{
			for (const auto& boundTex : key.m_BoundTextures)
			{
				vk::DescriptorImageInfo& info = m_ImageInfos.emplace_back();
				auto& tex = g_TextureManager.GetTexture(boundTex ? boundTex : g_TextureManager.GetStdTextureHandle(TEXTURE_BLACK));
				info.imageLayout = tex.GetDefaultLayout();
				info.imageView = tex.FindOrCreateView();
			}
		}
		else
		{
			// Can't have zero-sized texture arrays unfortunately
			write.descriptorCount = 1;
			vk::DescriptorImageInfo& info = m_ImageInfos.emplace_back();
			auto& tex = g_TextureManager.GetTexture(g_TextureManager.GetStdTextureHandle(TEXTURE_BLACK));
			info.imageLayout = tex.GetDefaultLayout();
			info.imageView = tex.FindOrCreateView();
		}

		write.pImageInfo = m_ImageInfos.data();
	}

	// Samplers
	{
		vk::WriteDescriptorSet& write = m_Writes.emplace_back();
		write.descriptorType = vk::DescriptorType::eSampler;
		write.dstSet = m_DescriptorSet.get();
		write.dstBinding = TF2Vulkan::Shaders::BINDING_SAMPLERS;
		write.descriptorCount = key.m_BoundSamplers.size();
		if (write.descriptorCount > 0)
		{
			for (const auto& boundSampler : key.m_BoundSamplers)
			{
				vk::DescriptorImageInfo& info = m_SamplerInfos.emplace_back();
				info.sampler = s_SMVulkan.FindOrCreateSampler(boundSampler).m_Sampler.get();
			}
		}
		else
		{
			// Can't have zero-sized sampler arrays unfortunately
			write.descriptorCount = 1;
			vk::DescriptorImageInfo& info = m_SamplerInfos.emplace_back();
			info.sampler = s_SMVulkan.FindOrCreateSampler(SamplerSettings{}).m_Sampler.get();
		}

		write.pImageInfo = m_SamplerInfos.data();
	}

	device.updateDescriptorSets(m_Writes, nullptr);
}

const DescriptorSet& StateManagerVulkan::FindOrCreateDescriptorSet(const DescriptorSetKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToDescSets.try_emplace(key, key).first->second;
}

void StateManagerVulkan::ApplyDescriptorSets(const Pipeline& pipeline,
	const LogicalDynamicState& dynamicState, IVulkanCommandBuffer& buf)
{
	const auto& layouts = pipeline.m_Layout->m_SetLayouts;
	assert(layouts.size() == 1);

	Util::InPlaceVector<vk::DescriptorSet, 8> descSets;
	Util::InPlaceVector<uint32_t, 16> dynamicOffsets;
	for (const auto& layout : layouts)
	{
		const auto& descSet = FindOrCreateDescriptorSet(DescriptorSetKey(*layout, dynamicState));
		descSets.push_back(descSet.m_DescriptorSet.get());

		for (const auto& write : descSet.m_Writes)
		{
			switch (write.descriptorType)
			{
				// Do nothing
			case vk::DescriptorType::eSampler:
			case vk::DescriptorType::eSampledImage:
				break;

			case vk::DescriptorType::eUniformBufferDynamic:
			{
				const auto bindingIndex = write.dstBinding;
				const auto& bufferEntry = dynamicState.m_UniformBuffers.at(bindingIndex);

				if (dynamicOffsets.size() <= bindingIndex)
					dynamicOffsets.resize(bindingIndex + 1);

				auto& dynamicOffset = dynamicOffsets.at(bindingIndex);
				dynamicOffset = bufferEntry ? bufferEntry.GetOffset() : 0;

				break;
			}

			default:
				throw VulkanException("Unexpected DescriptorType", EXCEPTION_DATA());
			}
		}
	}

	buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.m_Layout->m_Layout.get(), 0,
		{ descSets.size(), descSets.data() }, dynamicOffsets);
}

void StateManagerVulkan::ApplyState(VulkanStateID id, const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState, const IMesh& mesh, IVulkanCommandBuffer& buf)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	const auto& state = *m_IDsToPipelines.at(size_t(id));

	ApplyRenderPass(*state.m_RenderPass, buf);

	buf.bindPipeline(vk::PipelineBindPoint::eGraphics, state.m_Pipeline.get());

	ApplyDescriptorSets(state, dynamicState, buf);
}

DescriptorPool::DescriptorPool(const DescriptorPoolKey& key)
{
	LOG_FUNC();

	constexpr auto POOL_SIZE = 8192;

	// Sizes
	for (const auto& binding : key.m_Layout->m_Bindings)
	{
		if (binding.descriptorCount <= 0)
			continue;

		auto& size = m_Sizes.emplace_back();
		size.descriptorCount = binding.descriptorCount * POOL_SIZE;
		assert(size.descriptorCount > 0);
		size.type = binding.descriptorType;
	}

	// Descriptor pool
	{
		auto& ci = m_CreateInfo;
		AttachVector(ci.pPoolSizes, ci.poolSizeCount, m_Sizes);

		ci.maxSets = POOL_SIZE;
		ci.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		auto [device, lock] = g_ShaderDevice.GetVulkanDevice().locked();
		m_DescriptorPool = device.createDescriptorPoolUnique(ci,
			g_ShaderDeviceMgr.GetAllocationCallbacks());
		g_ShaderDevice.SetDebugName(m_DescriptorPool, "TF2Vulkan Descriptor Pool 0x%zX", Util::hash_value(key));
	}
}

const DescriptorPool& StateManagerVulkan::FindOrCreateDescriptorPool(const DescriptorPoolKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToDescPools.try_emplace(key, key).first->second;
}

const RenderPass& StateManagerVulkan::FindOrCreateRenderPass(const RenderPassKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToRenderPasses.try_emplace(key, key).first->second;
}

const PipelineLayout& StateManagerVulkan::FindOrCreatePipelineLayout(const PipelineLayoutKey& key)
{
	std::lock_guard lock(m_Mutex);
	return m_StatesToLayouts.try_emplace(key, key).first->second;
}

#include "interface/IMaterialInternal.h"

VulkanStateID StateManagerVulkan::FindOrCreateState(
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState,
	const IMesh& mesh)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	const PipelineKey key(staticState, dynamicState, mesh);

	auto lowerBound = m_StatesToPipelines.lower_bound(key);
	if (lowerBound != m_StatesToPipelines.end() && lowerBound->first == key)
		return lowerBound->second.m_ID;

	auto newEntry = m_StatesToPipelines.emplace_hint(lowerBound,
		key, Pipeline{ key, FindOrCreatePipelineLayout(key), FindOrCreateRenderPass(key) });

	Util::SafeConvert(m_IDsToPipelines.size(), newEntry->second.m_ID);
	m_IDsToPipelines.push_back(&newEntry->second);

	return newEntry->second.m_ID;
}

PipelineKey::PipelineKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState, const IMesh& mesh) :

	RenderPassKey(staticState, dynamicState),
	PipelineLayoutKey(staticState, dynamicState, mesh),

	m_DepthCompareFunc(dynamicState.m_ForceDepthFuncEquals ? SHADER_DEPTHFUNC_EQUAL : staticState.m_DepthCompareFunc),
	m_DepthTest(staticState.m_DepthTest),
	m_DepthWrite(staticState.m_DepthWrite),

	m_RSBackFaceCulling(staticState.m_RSBackFaceCulling),
	m_RSPolyMode(staticState.m_RSFrontFacePolyMode),

	m_OMAlphaBlending(staticState.m_OMAlphaBlending),
	m_OMSrcFactor(staticState.m_OMSrcFactor),
	m_OMDstFactor(staticState.m_OMDstFactor),

	m_Viewports(dynamicState.m_Viewports)
{
	assert((staticState.m_RSFrontFacePolyMode == staticState.m_RSBackFacePolyMode)
		|| staticState.m_RSBackFaceCulling);

	if (m_Viewports.empty())
	{
		auto& vp = m_Viewports.emplace_back();
		int bbWidth, bbHeight;
		g_ShaderDevice.GetBackBufferDimensions(bbWidth, bbHeight);
		vp.Init(0, 0, bbWidth, bbHeight);
	}

	if (staticState.m_OMDepthRT < 0 || (!m_DepthTest && !m_DepthWrite))
	{
		// Normalize all these so they don't affect the hash
		m_OMDepthRT = -1;
		m_DepthCompareFunc = SHADER_DEPTHFUNC_ALWAYS;
		m_DepthTest = false;
		m_DepthWrite = false;
	}
}

PipelineLayoutKey::PipelineLayoutKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState, const IMesh& mesh) :
	DescriptorSetLayoutKey(staticState, dynamicState),

	m_VSShaderInstance(dynamicState.m_VSShader),
	m_VSVertexFormat(mesh.GetVertexFormat()),

	m_PSShaderInstance(dynamicState.m_PSShader)
{
}

constexpr RenderPassKey::RenderPassKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_OMColorRTs{ staticState.m_OMColorRTs[0], staticState.m_OMColorRTs[1], staticState.m_OMColorRTs[2], staticState.m_OMColorRTs[3] },
	m_OMDepthRT(staticState.m_OMDepthRT)
{
	assert(m_OMColorRTs[0] >= 0 || m_OMColorRTs[1] >= 0 || m_OMColorRTs[2] >= 0 || m_OMColorRTs[3] >= 0);
}

FramebufferKey::RTRef::RTRef(IShaderAPITexture* tex) :
	m_ImageView(tex ? tex->FindOrCreateView() : nullptr),
	m_Extent(tex ? ToExtent2D(tex->GetImageCreateInfo().extent) : vk::Extent2D{})
{
}

FramebufferKey::FramebufferKey(const RenderPass& rp) :
	m_OMColorRTs
	{
		TryFindTexture(rp.m_Key.m_OMColorRTs[0]),
		TryFindTexture(rp.m_Key.m_OMColorRTs[1]),
		TryFindTexture(rp.m_Key.m_OMColorRTs[2]),
		TryFindTexture(rp.m_Key.m_OMColorRTs[3]),
	},
	m_OMDepthRT(TryFindTexture(rp.m_Key.m_OMDepthRT, true)),
	m_RenderPass(&rp)
{
	assert(!!m_OMColorRTs[0] || !!m_OMColorRTs[1] || !!m_OMColorRTs[2] || !!m_OMColorRTs[3]);
	//if (!m_OMColorRTs[0] && !m_OMColorRTs[1] && !m_OMColorRTs[2] && !m_OMColorRTs[3])
	//	m_OMColorRTs[0] = TryFindTexture(0);
}

void Pipeline::FixupPointers()
{
	for (auto& ci : m_ShaderStageCIs)
		ci.FixupPointers();

	auto& ci = m_CreateInfo;
	ci.pVertexInputState = &m_VertexInputStateCI;
	ci.pInputAssemblyState = &m_InputAssemblyStateCI;
	ci.pViewportState = &m_ViewportStateCI;
	ci.pRasterizationState = &m_RasterizationStateCI;
	ci.pMultisampleState = &m_MultisampleStateCI;
	ci.pColorBlendState = &m_ColorBlendStateCI;
	ci.layout = m_Layout->m_Layout.get();
	ci.renderPass = m_RenderPass->m_RenderPass.get();
}

DescriptorSetKey::UBRef::UBRef(const BufferPoolEntry& buf)
{
	if (buf)
	{
		auto& pool = buf.GetPool();
		const auto bufInfo = static_cast<const IBufferPoolInternal&>(pool).GetBufferInfo(buf);
		m_Buffer = bufInfo.m_Buffer;
		m_Length = bufInfo.m_Size;
	}
}

template<size_t size, size_t... Is>
static std::array<DescriptorSetKey::UBRef, size> ToUBRefArray(
	const std::array<BufferPoolEntry, size>& bufs, std::index_sequence<Is...>)
{
	return std::array<DescriptorSetKey::UBRef, size>{ DescriptorSetKey::UBRef(bufs[Is])... };
}

template<size_t size>
static std::array<DescriptorSetKey::UBRef, size> ToUBRefArray(const std::array<BufferPoolEntry, size>& bufs)
{
	return ToUBRefArray(bufs, std::make_index_sequence<size>{});
}

DescriptorSetKey::DescriptorSetKey(const DescriptorSetLayout& layout, const LogicalDynamicState& dynamicState) :
	m_Layout(&layout),
	m_UniformBuffers{ ToUBRefArray(dynamicState.m_UniformBuffers) },
	m_BoundTextures(dynamicState.m_BoundTextures),
	m_BoundSamplers(dynamicState.m_BoundSamplers)
{
}

template<size_t size>
static Util::InPlaceVector<uint32_t, size> GetOccupiedIndices(const std::array<BufferPoolEntry, size> & srcUBs)
{
	Util::InPlaceVector<uint32_t, size> retVal;

	for (uint32_t i = 0; i < size; i++)
	{
		if (srcUBs[i])
			retVal.push_back(i);
	}

	return retVal;
}

DescriptorSetLayoutKey::DescriptorSetLayoutKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :
	m_Texture2DCount(dynamicState.m_BoundTextures.size()),
	m_SamplerCount(dynamicState.m_BoundSamplers.size()),
	m_UniformBuffers(GetOccupiedIndices(dynamicState.m_UniformBuffers))
{
#ifdef _DEBUG
	// Both 0 or both > 0
	assert(!m_Texture2DCount == !m_SamplerCount);

	vk::SpecializationInfo specInfoPS, specInfoVS;
	dynamicState.m_VSShader->GetSpecializationInfo(specInfoVS);
	dynamicState.m_PSShader->GetSpecializationInfo(specInfoPS);

	uint32_t tex2DCountVS, tex2DCountPS, smpCountVS, smpCountPS;
	dynamicState.m_VSShader->GetResourceCounts(tex2DCountVS, smpCountVS);
	dynamicState.m_PSShader->GetResourceCounts(tex2DCountPS, smpCountPS);

	assert(tex2DCountVS == tex2DCountPS);
	assert(smpCountVS == smpCountPS);

	assert(tex2DCountVS == m_Texture2DCount);
	assert(smpCountVS == m_SamplerCount);
#endif
}
