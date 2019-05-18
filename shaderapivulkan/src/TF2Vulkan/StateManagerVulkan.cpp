 #include "interface/internal/IShaderAPIInternal.h"
#include "IShaderTextureManager.h"
#include "IStateManagerVulkan.h"
#include "LogicalState.h"
#include "MaterialSystemHardwareConfig.h"
#include "SamplerSettings.h"
#include "interface/internal/IShaderDeviceMgrInternal.h"
#include "interface/internal/IShaderDeviceInternal.h"
#include "interface/internal/IShaderInternal.h"
#include "interface/internal/IUniformBufferPoolInternal.h"
#include "shaders/VulkanShaderManager.h"
#include "VulkanFactories.h"

#include <TF2Vulkan/Util/AutoInit.h>
#include <TF2Vulkan/Util/MemoryPool.h>
#include <TF2Vulkan/Util/shaderapi_ishaderdynamic.h>
#include <TF2Vulkan/Util/std_array.h>

#include <stdshader_vulkan/ShaderData.h>

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
	struct PipelineLayoutKey
	{
		constexpr PipelineLayoutKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_STRONG_ORDERING_OPERATOR(PipelineLayoutKey);

		// TODO: Pipeline layout only needs to know shader groups, not specific instances
		const IShaderInstanceInternal* m_VSShader;
		VertexFormat m_VSVertexFormat;

		const IShaderInstanceInternal* m_PSShader;
	};
}

STD_HASH_DEFINITION(PipelineLayoutKey,
	v.m_VSShader,
	v.m_VSVertexFormat,

	v.m_PSShader
);

namespace
{
	struct PipelineKey final : RenderPassKey, PipelineLayoutKey
	{
		PipelineKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
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
	v.m_OMColorRTs
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
	struct Sampler final
	{
		vk::SamplerCreateInfo m_CreateInfo;
		vk::UniqueSampler m_Sampler;

		void FixupPointers();
		bool operator!() const { return !m_Sampler; }
	};

	struct DescriptorPool final
	{
		std::vector<vk::DescriptorPoolSize> m_Sizes;

		vk::DescriptorPoolCreateInfo m_CreateInfo;
		vk::UniqueDescriptorPool m_DescriptorPool;

		void FixupPointers();
		bool operator!() const { return !m_DescriptorPool; }
	};

	struct DescriptorSetLayout final
	{
		std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;

		vk::DescriptorSetLayoutCreateInfo m_CreateInfo;
		vk::UniqueDescriptorSetLayout m_Layout;

		void FixupPointers();
		bool operator!() const { return !m_Layout; }
	};

	struct PipelineLayout final
	{
		std::vector<DescriptorSetLayout> m_SetLayouts;
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
		RenderPass(RenderPassKey&& key) : m_Key(key) {}

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
		std::vector<vk::ImageView> m_Attachments;

		vk::FramebufferCreateInfo m_CreateInfo;
		vk::UniqueFramebuffer m_Framebuffer;

		void FixupPointers();
		bool operator!() const { return !m_Framebuffer; }
	};

	struct ShaderStageCreateInfo
	{
		const IVulkanShader* m_Shader = nullptr;

		vk::SpecializationInfo m_SpecializationInfo;
		vk::PipelineShaderStageCreateInfo m_CreateInfo;

		void FixupPointers();
	};

	struct Pipeline final
	{
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

	class StateManagerVulkan final : public IStateManagerVulkan, Util::IAutoInit<IShaderDeviceInternal>
	{
	public:
		void ApplyState(VulkanStateID stateID,
			const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState,
			IVulkanCommandBuffer& buf) override;

		VulkanStateID FindOrCreateState(const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState) override;

		void AutoInit() override;

	private:
		void ApplyRenderPass(const RenderPass& renderPass, IVulkanCommandBuffer& buf);
		void ApplyDescriptorSets(const Pipeline& pipeline,
			const LogicalDynamicState& dynamicState, IVulkanCommandBuffer& buf);

		const DescriptorPool& FindOrCreateDescriptorPool(const DescriptorPoolKey& key);
		const PipelineLayout& FindOrCreatePipelineLayout(const PipelineLayoutKey& key);
		const RenderPass& FindOrCreateRenderPass(const RenderPassKey& key);
		const Framebuffer& FindOrCreateFramebuffer(const FramebufferKey& key);
		const Sampler& FindOrCreateSampler(const SamplerKey& sampler);

		Pipeline CreatePipeline(const PipelineKey& key,
			const PipelineLayout& layout,
			const RenderPass& renderPass) const;

		std::recursive_mutex m_Mutex;

		std::unordered_map<PipelineKey, Pipeline> m_StatesToPipelines;
		std::vector<const Pipeline*> m_IDsToPipelines;
		std::unordered_map<PipelineLayoutKey, PipelineLayout> m_StatesToLayouts;
		std::unordered_map<RenderPassKey, RenderPass> m_StatesToRenderPasses;
		std::unordered_map<FramebufferKey, Framebuffer> m_StatesToFramebuffers;
		std::unordered_map<DescriptorPoolKey, DescriptorPool> m_StatesToDescPools;
		std::unordered_map<SamplerKey, Sampler> m_StatesToSamplers;

		vma::AllocatedBuffer m_DummyUniformBuffer;
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

static ShaderStageCreateInfo CreateStageInfo(
	const IShaderInstanceInternal& shader, vk::ShaderStageFlagBits type)
{
	ShaderStageCreateInfo retVal;

	auto& ci = retVal.m_CreateInfo;

	retVal.m_Shader = &shader.GetGroup().GetVulkanShader();

	shader.GetSpecializationInfo(retVal.m_SpecializationInfo);

	ci.stage = type;
	ci.module = retVal.m_Shader->GetModule();
	ci.pName = "main"; // Shader entry point
	ci.pSpecializationInfo = &retVal.m_SpecializationInfo;

	return retVal;
}

static void CreateBindings(DescriptorSetLayout& layout, const IShaderGroupInternal& shader)
{
	const auto& reflectionData = shader.GetVulkanShader().GetReflectionData();

	auto& bindings = layout.m_Bindings;

	const auto TryAddBinding = [&bindings](uint32_t binding, vk::DescriptorType type, vk::ShaderStageFlags stages)
	{
		bool found = false;

		for (auto& existingBinding : bindings)
		{
			if (existingBinding.binding != binding ||
				existingBinding.descriptorType != type)
			{
				continue;
			}

			existingBinding.stageFlags |= stages;
			found = true;
		}

		if (!found)
		{
			auto& newBinding = bindings.emplace_back();
			newBinding.binding = binding;
			newBinding.descriptorCount = 1;
			newBinding.descriptorType = type;
			newBinding.stageFlags = stages;
		}
	};

	// Samplers
	for (const auto& samplerIn : reflectionData.m_Samplers)
		TryAddBinding(samplerIn.m_Binding, vk::DescriptorType::eSampler, reflectionData.m_ShaderStage);

	// Textures
	for (const auto& textureIn : reflectionData.m_Textures)
		TryAddBinding(textureIn.m_Binding, vk::DescriptorType::eSampledImage, reflectionData.m_ShaderStage);

	// Constant buffers
	for (const auto& cbufIn : reflectionData.m_UniformBuffers)
		TryAddBinding(cbufIn.m_Binding, vk::DescriptorType::eUniformBufferDynamic, reflectionData.m_ShaderStage);
}

static DescriptorSetLayout CreateDescriptorSetLayout(const PipelineLayoutKey& key)
{
	DescriptorSetLayout retVal;

	// Bindings
	CreateBindings(retVal, key.m_VSShader->GetGroup());
	CreateBindings(retVal, key.m_PSShader->GetGroup());

	// Descriptor set layout
	{
		auto& ci = retVal.m_CreateInfo;

		AttachVector(ci.pBindings, ci.bindingCount, retVal.m_Bindings);

		retVal.m_Layout = g_ShaderDevice.GetVulkanDevice().createDescriptorSetLayoutUnique(ci);
		g_ShaderDevice.SetDebugName(retVal.m_Layout, "test descriptor set layout");
	}

	return retVal;
}

static std::vector<DescriptorSetLayout> CreateDescriptorSetLayouts(
	const PipelineLayoutKey& key)
{
	std::vector<DescriptorSetLayout> retVal;

	retVal.push_back(CreateDescriptorSetLayout(key));

	return retVal;
}

static PipelineLayout CreatePipelineLayout(const PipelineLayoutKey& key)
{
	PipelineLayout retVal;

	// Descriptor set layouts
	retVal.m_SetLayouts = CreateDescriptorSetLayouts(key);

	// Pipeline Layout
	{
		auto& ci = retVal.m_CreateInfo;

		std::vector<vk::DescriptorSetLayout> setLayouts;
		for (auto& sl : retVal.m_SetLayouts)
			setLayouts.push_back(sl.m_Layout.get());

		AttachVector(ci.pSetLayouts, ci.setLayoutCount, setLayouts);
		AttachVector(ci.pPushConstantRanges, ci.pushConstantRangeCount, retVal.m_PushConstantRanges);
		retVal.m_Layout = g_ShaderDevice.GetVulkanDevice().createPipelineLayoutUnique(ci);

		char buf[128];
		sprintf_s(buf, "TF2Vulkan Pipeline Layout 0x%zX", Util::hash_value(key));
		g_ShaderDevice.SetDebugName(retVal.m_Layout, buf);
	}

	return retVal;
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

static RenderPass CreateRenderPass(const RenderPassKey& key)
{
	RenderPass retVal{ RenderPassKey(key) };

	// Subpass 0
	{
		Subpass& sp = retVal.m_Subpasses.emplace_back();

		// Color attachments
		{
			for (auto& colorTexID : key.m_OMColorRTs)
			{
				if (colorTexID < 0)
					continue;

				auto& colorTex = FindTexture(colorTexID);

				vk::AttachmentDescription& att = retVal.m_Attachments.emplace_back();
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
				Util::SafeConvert(retVal.m_Attachments.size() - 1, attRef.attachment);
			}
		}

		// Depth attachments
		if (key.m_OMDepthRT >= 0)
		{
			vk::AttachmentDescription& att = retVal.m_Attachments.emplace_back();

			att.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			att.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			att.samples = vk::SampleCountFlagBits::e1;
			att.loadOp = vk::AttachmentLoadOp::eLoad; // TODO: Switch to eClear when we call ClearBuffers()
			att.storeOp = vk::AttachmentStoreOp::eStore;
			att.stencilLoadOp = vk::AttachmentLoadOp::eLoad; // TODO: Switch to eClear when we call ClearBuffers()
			att.stencilStoreOp = vk::AttachmentStoreOp::eStore;

			att.format = FindTexture(key.m_OMDepthRT, true).GetImageCreateInfo().format;

			vk::AttachmentReference& attRef = sp.m_DepthStencilAttachment;
			attRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			Util::SafeConvert(retVal.m_Attachments.size() - 1, attRef.attachment);

			sp.m_CreateInfo.pDepthStencilAttachment = &sp.m_DepthStencilAttachment;
		}

		AttachVector(sp.m_CreateInfo.pInputAttachments, sp.m_CreateInfo.inputAttachmentCount, sp.m_InputAttachments);
		AttachVector(sp.m_CreateInfo.pColorAttachments, sp.m_CreateInfo.colorAttachmentCount, sp.m_ColorAttachments);
	}

	// Render pass
	{
		auto& ci = retVal.m_CreateInfo;

		AttachVector(ci.pAttachments, ci.attachmentCount, retVal.m_Attachments);
		AttachVector(ci.pDependencies, ci.dependencyCount, retVal.m_Dependencies);

		std::vector<vk::SubpassDescription> subpassTemp;
		for (auto& sp : retVal.m_Subpasses)
			subpassTemp.push_back(sp.m_CreateInfo);

		AttachVector(ci.pSubpasses, ci.subpassCount, subpassTemp);

		retVal.m_RenderPass = g_ShaderDevice.GetVulkanDevice().createRenderPassUnique(ci);

		char buf[128];
		sprintf_s(buf, "TF2Vulkan Render Pass 0x%zX", Util::hash_value(key));
		g_ShaderDevice.SetDebugName(retVal.m_RenderPass, buf);
	}

	return retVal;
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

Pipeline StateManagerVulkan::CreatePipeline(const PipelineKey& key, const PipelineLayout& layout,
	const RenderPass& renderPass) const
{
	Pipeline retVal;

	// Preconfigured objects
	retVal.m_Layout = &layout;
	retVal.m_RenderPass = &renderPass;

	// Shader stage create info(s)
	{
		auto& cis = retVal.m_ShaderStageCIs;
		cis.emplace_back(CreateStageInfo(*key.m_VSShader, vk::ShaderStageFlagBits::eVertex));
		cis.emplace_back(CreateStageInfo(*key.m_PSShader, vk::ShaderStageFlagBits::eFragment));
	}

	// Vertex input state create info
	{
		auto& attrs = retVal.m_VertexInputAttributeDescriptions;

		const auto& vertexShaderRefl = key.m_VSShader->GetGroup().GetVulkanShader().GetReflectionData();

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
			attrs.emplace_back(VIAD(input.m_Location, 1, vk::Format::eR32G32B32A32Sfloat));
		}

		auto & binds = retVal.m_VertexInputBindingDescriptions;
		binds.emplace_back(vk::VertexInputBindingDescription(0, totalVertexSize));

		// "Fake" binding with no real data
		binds.emplace_back(vk::VertexInputBindingDescription(1, sizeof(float) * 4, vk::VertexInputRate::eInstance));

		auto& ci = retVal.m_VertexInputStateCI;
		AttachVector(ci.pVertexAttributeDescriptions, ci.vertexAttributeDescriptionCount, attrs);
		AttachVector(ci.pVertexBindingDescriptions, ci.vertexBindingDescriptionCount, binds);
	}

	// Vertex input assembly state create info
	{
		auto& ci = retVal.m_InputAssemblyStateCI;
		ci.topology = vk::PrimitiveTopology::eTriangleList;
	}

	// Viewport/scissor state
	{
		// Viewport(s)
		{
			auto& ci = retVal.m_ViewportStateCI;
			for (const auto& vpIn : key.m_Viewports)
			{
				auto& vpOut = retVal.m_Viewports.emplace_back();
				Util::SafeConvert(vpIn.m_nWidth, vpOut.width);
				Util::SafeConvert(vpIn.m_nHeight, vpOut.height);
				Util::SafeConvert(vpIn.m_nTopLeftX, vpOut.x);
				Util::SafeConvert(vpIn.m_nTopLeftY, vpOut.y);
				Util::SafeConvert(vpIn.m_flMinZ, vpOut.minDepth);
				Util::SafeConvert(vpIn.m_flMaxZ, vpOut.maxDepth);
			}

			AttachVector(ci.pViewports, ci.viewportCount, retVal.m_Viewports);
		}

		// Scissor(s)
		{
			retVal.m_Scissor.extent.width = retVal.m_ViewportStateCI.pViewports[0].width;
			retVal.m_Scissor.extent.height = retVal.m_ViewportStateCI.pViewports[0].height;
			retVal.m_ViewportStateCI.pScissors = &retVal.m_Scissor;
			retVal.m_ViewportStateCI.scissorCount = 1;
		}
	}

	// Rasterization state create info
	{
		auto& ci = retVal.m_RasterizationStateCI;

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
		auto& ci = retVal.m_MultisampleStateCI;
		ci.rasterizationSamples = vk::SampleCountFlagBits::e1;
	}

	// Color blend state create info
	{
		auto& ci = retVal.m_ColorBlendStateCI;

		ci.setBlendConstants({ 1, 1, 1, 1 });

		auto& att = retVal.m_ColorBlendAttachmentStates.emplace_back();

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

		AttachVector(ci.pAttachments, ci.attachmentCount, retVal.m_ColorBlendAttachmentStates);
	}

	// Depth stencil state
	{
		auto& ci = retVal.m_DepthStencilStateCI;
		retVal.m_CreateInfo.pDepthStencilState = &ci;

		ci.depthTestEnable = key.m_DepthTest;
		ci.depthWriteEnable = key.m_DepthWrite;
		ci.depthCompareOp = ConvertCompareOp(key.m_DepthCompareFunc);
	}

	// Graphics pipeline
	{
		vk::GraphicsPipelineCreateInfo& ci = retVal.m_CreateInfo;

		retVal.FixupPointers();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for (const auto& stage : retVal.m_ShaderStageCIs)
			shaderStages.push_back(stage.m_CreateInfo);

		AttachVector(ci.pStages, ci.stageCount, shaderStages);

		// ci.subpass = 0;
		retVal.m_Pipeline = g_ShaderDevice.GetVulkanDevice().createGraphicsPipelineUnique(nullptr, ci);

		char buf[128];
		sprintf_s(buf, "TF2Vulkan Graphics Pipeline 0x%zX", Util::hash_value(key));
		g_ShaderDevice.SetDebugName(retVal.m_Pipeline, buf);
	}

	return retVal;
}

static Framebuffer CreateFramebuffer(const FramebufferKey& key)
{
	Framebuffer retVal;

	// Attachments
	uint32_t width = 0;// std::numeric_limits<uint32_t>::max();
	uint32_t height = 0;// width;

	std::string debugName = "COL{ ";
	// Color attachments
	{
		bool debugNameFirst = true;

		auto& atts = retVal.m_Attachments;
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
		retVal.m_Attachments.push_back(key.m_OMDepthRT.m_ImageView);

		debugName += " DEPTH { ";
		debugName += std::to_string((VkImageView)key.m_OMDepthRT.m_ImageView);
		debugName += " }";
	}

	// Framebuffer
	{
		auto& ci = retVal.m_CreateInfo;
		ci.width = width;
		ci.height = height;
		ci.layers = 1;

		AttachVector(ci.pAttachments, ci.attachmentCount, retVal.m_Attachments);

		ci.renderPass = key.m_RenderPass->m_RenderPass.get();

		retVal.m_Framebuffer = g_ShaderDevice.GetVulkanDevice().createFramebufferUnique(ci);
		assert(retVal.m_Framebuffer);
		g_ShaderDevice.SetDebugName(retVal.m_Framebuffer, debugName.c_str());
	}

	return retVal;
}

const Framebuffer& StateManagerVulkan::FindOrCreateFramebuffer(const FramebufferKey& key)
{
	std::lock_guard lock(m_Mutex);

	auto& fb = m_StatesToFramebuffers[key];
	if (!fb)
		fb = CreateFramebuffer(key);

	return fb;
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

static Sampler CreateSampler(const SamplerKey& key)
{
	Sampler retVal;

	auto& ci = retVal.m_CreateInfo;
	ci.addressModeU = ConvertAddressMode(key.m_Settings.m_WrapS);
	ci.addressModeV = ConvertAddressMode(key.m_Settings.m_WrapT);
	ci.addressModeW = ConvertAddressMode(key.m_Settings.m_WrapU);
	ci.minFilter = ConvertFilter(key.m_Settings.m_MinFilter);
	ci.magFilter = ConvertFilter(key.m_Settings.m_MagFilter);
	ci.anisotropyEnable = (key.m_Settings.m_MinFilter == SHADER_TEXFILTERMODE_ANISOTROPIC ||
		key.m_Settings.m_MagFilter == SHADER_TEXFILTERMODE_ANISOTROPIC);

	if (ConvertMipmapMode(key.m_Settings.m_MinFilter) == vk::SamplerMipmapMode::eLinear ||
		ConvertMipmapMode(key.m_Settings.m_MagFilter) == vk::SamplerMipmapMode::eLinear)
	{
		ci.mipmapMode = vk::SamplerMipmapMode::eLinear;
	}
	else
	{
		ci.mipmapMode = vk::SamplerMipmapMode::eNearest;
	}

	retVal.m_Sampler = g_ShaderDevice.GetVulkanDevice().createSamplerUnique(retVal.m_CreateInfo);
	{
		char buf[128];
		sprintf_s(buf, "TF2Vulkan Sampler 0x%zX", Util::hash_value(key));
		g_ShaderDevice.SetDebugName(retVal.m_Sampler, buf);
	}

	return retVal;
}

const Sampler& StateManagerVulkan::FindOrCreateSampler(const SamplerKey& key)
{
	std::lock_guard lock(m_Mutex);

	auto& sampler = m_StatesToSamplers[key];
	if (!sampler)
		sampler = CreateSampler(key);

	return sampler;
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

	//state.m_RenderPass->m_CreateInfo.

	// Hack!
	const auto& fb = FindOrCreateFramebuffer(renderPass);
	rpInfo.framebuffer = fb.m_Framebuffer.get();

	rpInfo.renderArea.extent.width = fb.m_CreateInfo.width;
	rpInfo.renderArea.extent.height = fb.m_CreateInfo.height;

	if (!buf.IsRenderPassActive(rpInfo, vk::SubpassContents::eInline))
	{
		if (buf.GetActiveRenderPass())
			buf.endRenderPass();

		buf.beginRenderPass(rpInfo, vk::SubpassContents::eInline);
	}
}

void StateManagerVulkan::ApplyDescriptorSets(const Pipeline& pipeline,
	const LogicalDynamicState& dynamicState, IVulkanCommandBuffer& buf)
{
	auto& device = g_ShaderDevice.GetVulkanDevice();

	const auto& layouts = pipeline.m_Layout->m_SetLayouts;
	assert(layouts.size() == 1);

	std::vector<vk::UniqueDescriptorSet> descriptorSets;
	std::vector<vk::WriteDescriptorSet> writes;
	std::vector<vk::CopyDescriptorSet> copies;
	std::vector<uint32_t> dynamicOffsets;
	std::forward_list<vk::DescriptorImageInfo> imageInfos;
	std::forward_list<vk::DescriptorBufferInfo> bufferInfos;
	for (const auto& layout : layouts)
	{
		auto& pool = FindOrCreateDescriptorPool(layout);

		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = pool.m_DescriptorPool.get();
		allocInfo.pSetLayouts = &layout.m_Layout.get();
		allocInfo.descriptorSetCount = 1;

		auto& newSet = descriptorSets.emplace_back(std::move(device.allocateDescriptorSetsUnique(allocInfo).at(0)));

		for (size_t i = 0; i < layout.m_Bindings.size(); i++)
		{
			const vk::DescriptorSetLayoutBinding &binding = layout.m_Bindings.at(i);

			auto& write = writes.emplace_back();
			write.descriptorType = binding.descriptorType;
			write.descriptorCount = binding.descriptorCount;
			write.dstBinding = binding.binding;
			write.dstSet = newSet.get();

			switch (write.descriptorType)
			{
			case vk::DescriptorType::eSampler:
			{
				auto& imgInfo = imageInfos.emplace_front();
				auto& sampler = FindOrCreateSampler(SamplerSettings{});
				imgInfo.sampler = sampler.m_Sampler.get();
				write.pImageInfo = &imgInfo;
				break;
			}
			case vk::DescriptorType::eSampledImage:
			{
				auto& imgInfo = imageInfos.emplace_front();
				auto& tex = g_TextureManager.TryGetTexture(
					dynamicState.m_BoundTextures.at(binding.binding - BINDING_TEXTURE_OFFSET),
					TEXTURE_BLACK);
				imgInfo.imageView = tex.FindOrCreateView();
				//imgInfo.sampler = FindOrCreateSampler(SamplerSettings{}).m_Sampler.get();
				imgInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				write.pImageInfo = &imgInfo;
				break;
			}
			case vk::DescriptorType::eUniformBufferDynamic:
			{
				const auto& bufferEntry = dynamicState.m_UniformBuffers.at(binding.binding);

				const bool isBufferBound = !!bufferEntry;

				vk::DescriptorBufferInfo& bufInfo = bufferInfos.emplace_front();
				write.pBufferInfo = &bufInfo;
				bufInfo.offset = 0;

				if (dynamicOffsets.size() <= binding.binding)
					dynamicOffsets.resize(binding.binding + 1);

				auto& dynamicOffset = dynamicOffsets.at(binding.binding);

				if (bufferEntry)
				{
					const auto& pool = static_cast<const IUniformBufferPoolInternal&>(bufferEntry.GetPool());
					bufInfo.buffer = pool.GetBackingBuffer();
					bufInfo.range = pool.GetChildBufferSize();
					dynamicOffset = bufferEntry.GetOffset();
				}
				else
				{
					bufInfo.buffer = m_DummyUniformBuffer.GetBuffer();
					bufInfo.range = VK_WHOLE_SIZE;
					dynamicOffset = 0;
				}

				break;
			}

			default:
				throw VulkanException("Unexpected DescriptorType", EXCEPTION_DATA());
			}
		}
	}

	device.updateDescriptorSets(writes, copies);

	TF2VULKAN_MAKE_NONUNIQUE_STACK_COPY(rawSets, descriptorSets);

	buf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.m_Layout->m_Layout.get(), 0, { descriptorSets.size(), rawSets }, dynamicOffsets);
	buf.AddResource(std::move(descriptorSets));
}

void StateManagerVulkan::ApplyState(VulkanStateID id, const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState, IVulkanCommandBuffer& buf)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	auto pixScope = buf.DebugRegionBegin("StateManagerVulkan::ApplyState(%zu)", Util::UValue(id));

	const auto& state = *m_IDsToPipelines.at(size_t(id));

	buf.bindPipeline(vk::PipelineBindPoint::eGraphics, state.m_Pipeline.get());

	ApplyRenderPass(*state.m_RenderPass, buf);

	ApplyDescriptorSets(state, dynamicState, buf);
}

static DescriptorPool CreateDescriptorPool(const DescriptorPoolKey& key)
{
	LOG_FUNC();
	DescriptorPool retVal;

	constexpr auto POOL_SIZE = 1024;

	// Sizes
	for (const auto& binding : key.m_Layout->m_Bindings)
	{
		auto& size = retVal.m_Sizes.emplace_back();
		size.descriptorCount = binding.descriptorCount * POOL_SIZE;
		size.type = binding.descriptorType;
	}

	// Descriptor pool
	{
		auto& ci = retVal.m_CreateInfo;
		AttachVector(ci.pPoolSizes, ci.poolSizeCount, retVal.m_Sizes);

		ci.maxSets = POOL_SIZE;
		ci.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		retVal.m_DescriptorPool = g_ShaderDevice.GetVulkanDevice().createDescriptorPoolUnique(ci);
		{
			char buf[128];
			sprintf_s(buf, "TF2Vulkan Descriptor Pool 0x%zX", Util::hash_value(key));
			g_ShaderDevice.SetDebugName(retVal.m_DescriptorPool, buf);
		}
	}

	return retVal;
}

const DescriptorPool& StateManagerVulkan::FindOrCreateDescriptorPool(const DescriptorPoolKey& key)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	auto& pool = m_StatesToDescPools[key];
	if (!pool)
		pool = CreateDescriptorPool(key);

	return pool;
}

const RenderPass& StateManagerVulkan::FindOrCreateRenderPass(const RenderPassKey& key)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	if (auto found = m_StatesToRenderPasses.find(key); found != m_StatesToRenderPasses.end())
	{
		assert(!!found->second);
		return found->second;
	}

	return m_StatesToRenderPasses.emplace(key, CreateRenderPass(key)).first->second;
}

const PipelineLayout& StateManagerVulkan::FindOrCreatePipelineLayout(const PipelineLayoutKey& key)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	auto& layout = m_StatesToLayouts[key];
	if (!layout)
		layout = CreatePipelineLayout(key);

	return layout;
}

#include "interface/IMaterialInternal.h"

VulkanStateID StateManagerVulkan::FindOrCreateState(
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	const PipelineKey key(staticState, dynamicState);
	auto& pl = m_StatesToPipelines[key];
	if (!pl)
	{
		pl = CreatePipeline(key,
			FindOrCreatePipelineLayout(key),
			FindOrCreateRenderPass(key));

		Util::SafeConvert(m_IDsToPipelines.size(), pl.m_ID);
		m_IDsToPipelines.push_back(&pl);
	}

	return pl.m_ID;
}

PipelineKey::PipelineKey(
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState) :

	RenderPassKey(staticState, dynamicState),
	PipelineLayoutKey(staticState, dynamicState),

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

constexpr PipelineLayoutKey::PipelineLayoutKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_VSShader(dynamicState.m_VSShader),
	m_VSVertexFormat(staticState.m_VSVertexFormat),

	m_PSShader(dynamicState.m_PSShader)
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

void ShaderStageCreateInfo::FixupPointers()
{
	auto& specInfo = m_SpecializationInfo;
	m_CreateInfo.pSpecializationInfo = &specInfo;
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

void StateManagerVulkan::AutoInit()
{
	m_DummyUniformBuffer = Factories::BufferFactory{}
		.SetDebugName("Dummy uniform buffer")
		.SetSize(g_ShaderDeviceMgr.GetAdapterLimits().maxUniformBufferRange)
		.SetUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.Create();
}
