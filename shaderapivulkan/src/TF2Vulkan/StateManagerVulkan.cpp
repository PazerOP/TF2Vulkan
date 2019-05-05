 #include "interface/internal/IShaderAPIInternal.h"
#include "IStateManagerVulkan.h"
#include "LogicalState.h"
#include "MaterialSystemHardwareConfig.h"
#include "ShaderDevice.h"
#include "shaders/VulkanShaderManager.h"

#include <TF2Vulkan/Util/MemoryPool.h>
#include <TF2Vulkan/Util/std_array.h>

#include <stdshader_dx9_tf2vulkan/ShaderShared.h>

#undef min
#undef max

#include <mutex>
#include <unordered_map>

using namespace TF2Vulkan;

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

		CUtlSymbolDbg m_VSName;
		int m_VSStaticIndex;
		VertexFormat m_VSVertexFormat;

		CUtlSymbolDbg m_PSName;
		int m_PSStaticIndex;
	};
}

STD_HASH_DEFINITION(PipelineLayoutKey,
	v.m_VSName,
	v.m_VSStaticIndex,
	v.m_VSVertexFormat,

	v.m_PSName,
	v.m_PSStaticIndex
);

namespace
{
	struct PipelineKey final : RenderPassKey, PipelineLayoutKey
	{
		constexpr PipelineKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_STRONG_ORDERING_OPERATOR(PipelineKey);

		ShaderDepthFunc_t m_DepthCompareFunc;
		bool m_DepthTest;
		bool m_DepthWrite;

		bool m_RSBackFaceCulling;
		ShaderPolyMode_t m_RSPolyMode;

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

		RTRef m_OMDepthRT;
		std::array<RTRef, 4> m_OMColorRTs;

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
	struct DescriptorSetLayout final
	{
		std::vector<vk::DescriptorSetLayoutBinding> m_Bindings;

		vk::DescriptorSetLayoutCreateInfo m_CreateInfo;
		vk::UniqueDescriptorSetLayout m_Layout;
	};

	struct PipelineLayout final
	{
		std::vector<DescriptorSetLayout> m_SetLayouts;
		std::vector<vk::PushConstantRange> m_PushConstantRanges;

		vk::PipelineLayoutCreateInfo m_CreateInfo;
		vk::UniquePipelineLayout m_Layout;

		bool operator!() const { return !m_Layout; }
	};

	struct Subpass final
	{
		std::vector<vk::AttachmentReference> m_InputAttachments;
		std::vector<vk::AttachmentReference> m_ColorAttachments;
		vk::AttachmentReference m_DepthStencilAttachment;

		vk::SubpassDescription m_CreateInfo;
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

		bool operator!() const { return !m_RenderPass; }
	};

	struct Framebuffer final
	{
		std::vector<vk::ImageView> m_Attachments;

		vk::FramebufferCreateInfo m_CreateInfo;
		vk::UniqueFramebuffer m_Framebuffer;

		bool operator!() const { return !m_Framebuffer; }
	};

	struct ShaderStageCreateInfo
	{
		const IVulkanShaderManager::IShader* m_Shader = nullptr;
		vk::PipelineShaderStageCreateInfo m_CreateInfo;
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

		bool operator!() const { return !m_Pipeline; }
		VulkanStateID m_ID;
	};

	class StateManagerVulkan final : public IStateManagerVulkan
	{
	public:
		void ApplyState(VulkanStateID stateID, const vk::CommandBuffer& buf) override;

		VulkanStateID FindOrCreateState(
			LogicalShadowStateID staticID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState);

	private:
		const PipelineLayout& FindOrCreatePipelineLayout(const PipelineLayoutKey& key);
		const RenderPass& FindOrCreateRenderPass(const RenderPassKey& key);
		const Framebuffer& FindOrCreateFramebuffer(const FramebufferKey& key);

		Pipeline CreatePipeline(const PipelineKey& key,
			const PipelineLayout& layout,
			const RenderPass& renderPass) const;

		std::recursive_mutex m_Mutex;

		std::unordered_map<PipelineKey, Pipeline> m_StatesToPipelines;
		std::vector<const Pipeline*> m_IDsToPipelines;
		std::unordered_map<PipelineLayoutKey, PipelineLayout> m_StatesToLayouts;
		std::unordered_map<RenderPassKey, RenderPass> m_StatesToRenderPasses;
		std::unordered_map<FramebufferKey, Framebuffer> m_StatesToFramebuffers;
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
	const CUtlSymbolDbg& name, int staticIndex, vk::ShaderStageFlagBits type)
{
	ShaderStageCreateInfo retVal;
	retVal.m_CreateInfo.stage = type;

	retVal.m_Shader = &g_ShaderManager.FindOrCreateShader(
		name, staticIndex);

	retVal.m_CreateInfo.module = retVal.m_Shader->GetModule();

	retVal.m_CreateInfo.pName = "main"; // Shader entry point

	return retVal;
}

static void CreateBindings(DescriptorSetLayout& layout, const CUtlSymbolDbg& shaderName, int shaderStaticIndex)
{
	const auto& reflectionData =
		g_ShaderManager.FindOrCreateShader(shaderName, shaderStaticIndex).GetReflectionData();

	auto& bindings = layout.m_Bindings;

	// Samplers
	for (const auto& samplerIn : reflectionData.m_Samplers)
	{
		auto& samplerOut = bindings.emplace_back();
		samplerOut.binding = samplerIn.m_Binding;
		samplerOut.descriptorCount = 1;
		samplerOut.descriptorType = vk::DescriptorType::eSampler;
		samplerOut.stageFlags = reflectionData.m_ShaderStage;
	}

	// Textures
	for (const auto& textureIn : reflectionData.m_Textures)
	{
		auto& textureOut = bindings.emplace_back();
		textureOut.binding = textureIn.m_Binding;
		textureOut.descriptorCount = 1;
		textureOut.descriptorType = vk::DescriptorType::eSampledImage;
		textureOut.stageFlags = reflectionData.m_ShaderStage;
	}

	// Constant buffers
	for (const auto& cbufIn : reflectionData.m_UniformBuffers)
	{
		auto& cbufOut = bindings.emplace_back();
		cbufOut.binding = cbufIn.m_Binding;
		cbufOut.descriptorCount = 1;
		cbufOut.descriptorType = vk::DescriptorType::eUniformBuffer;
		cbufOut.stageFlags = reflectionData.m_ShaderStage;
	}
}

static DescriptorSetLayout CreateDescriptorSetLayout(const PipelineLayoutKey& key)
{
	DescriptorSetLayout retVal;

	// Bindings
	CreateBindings(retVal, key.m_VSName, key.m_VSStaticIndex);
	CreateBindings(retVal, key.m_PSName, key.m_PSStaticIndex);

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

	return &g_ShaderAPIInternal.GetTexture(handle);
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
				att.initialLayout = vk::ImageLayout::eUndefined;
				att.finalLayout = vk::ImageLayout::ePresentSrcKHR;
				att.samples = vk::SampleCountFlagBits::e1;
				att.loadOp = vk::AttachmentLoadOp::eClear;
				att.storeOp = vk::AttachmentStoreOp::eStore;
				att.stencilLoadOp = vk::AttachmentLoadOp::eClear;
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

			att.initialLayout = vk::ImageLayout::eUndefined;
			att.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			att.samples = vk::SampleCountFlagBits::e1;
			att.loadOp = vk::AttachmentLoadOp::eClear;
			att.storeOp = vk::AttachmentStoreOp::eStore;
			att.stencilLoadOp = vk::AttachmentLoadOp::eClear;
			att.stencilStoreOp = vk::AttachmentStoreOp::eStore;

			att.format = FindTexture(key.m_OMDepthRT).GetImageCreateInfo().format;

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
		cis.emplace_back(CreateStageInfo(key.m_VSName.String(), key.m_VSStaticIndex, vk::ShaderStageFlagBits::eVertex));
		cis.emplace_back(CreateStageInfo(key.m_PSName.String(), key.m_PSStaticIndex, vk::ShaderStageFlagBits::eFragment));
	}

	// Vertex input state create info
	{
		auto& attrs = retVal.m_VertexInputAttributeDescriptions;

		VertexFormat::Element vertexElements[VERTEX_ELEMENT_NUMELEMENTS];
		size_t totalVertexSize;
		const auto vertexElementCount = key.m_VSVertexFormat.GetVertexElements(vertexElements, std::size(vertexElements), &totalVertexSize);
		using VIAD = vk::VertexInputAttributeDescription;
		for (uint_fast8_t i = 0; i < vertexElementCount; i++)
		{
			const auto& vertexElement = vertexElements[i];
			attrs.emplace_back(VIAD(i, 0, vertexElement.m_Type->GetVKFormat(), vertexElement.m_Offset));
		}

		const auto maxVertexAttributes = g_MatSysConfig.MaxVertexAttributes();
		for (size_t i = 0; i < maxVertexAttributes; i++)
		{
			bool found = false;
			for (auto& attr : attrs)
			{
				if (attr.location == i)
				{
					found = true;
					break;
				}
			}

			if (found)
				continue;

			// Otherwise, insert an empty one
			attrs.emplace_back(VIAD(i, 1, vk::Format::eR32G32B32A32Sfloat));
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

		ci.lineWidth = 1; // default
		ci.cullMode = key.m_RSBackFaceCulling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone;
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

		auto& att = retVal.m_ColorBlendAttachmentStates.emplace_back();

		att.srcAlphaBlendFactor = att.srcColorBlendFactor = ConvertBlendFactor(key.m_OMSrcFactor);
		att.dstAlphaBlendFactor = att.dstColorBlendFactor = ConvertBlendFactor(key.m_OMDstFactor);

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

		ci.depthTestEnable = false;// key.m_DepthTest;
		ci.depthWriteEnable = key.m_DepthWrite;
		ci.depthCompareOp = ConvertCompareOp(key.m_DepthCompareFunc);
	}

	// Graphics pipeline
	{
		vk::GraphicsPipelineCreateInfo& ci = retVal.m_CreateInfo;

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for (const auto& stage : retVal.m_ShaderStageCIs)
			shaderStages.push_back(stage.m_CreateInfo);

		AttachVector(ci.pStages, ci.stageCount, shaderStages);

		ci.pVertexInputState = &retVal.m_VertexInputStateCI;
		ci.pInputAssemblyState = &retVal.m_InputAssemblyStateCI;
		ci.pViewportState = &retVal.m_ViewportStateCI;
		ci.pRasterizationState = &retVal.m_RasterizationStateCI;
		ci.pMultisampleState = &retVal.m_MultisampleStateCI;
		ci.pColorBlendState = &retVal.m_ColorBlendStateCI;
		ci.layout = retVal.m_Layout->m_Layout.get();
		ci.renderPass = retVal.m_RenderPass->m_RenderPass.get();

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
	uint32_t width = std::numeric_limits<uint32_t>::max();
	uint32_t height = width;

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

			width = std::min(width, colorRT.m_Extent.width);
			height = std::min(height, colorRT.m_Extent.height);
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

void StateManagerVulkan::ApplyState(VulkanStateID id, const vk::CommandBuffer& buf)
{
	LOG_FUNC();
	std::lock_guard lock(m_Mutex);

	PixScope pixScope(buf, "StateManagerVulkan::ApplyState(%zu)", Util::UValue(id));

	const auto& state = *m_IDsToPipelines.at(size_t(id));

	buf.bindPipeline(vk::PipelineBindPoint::eGraphics, state.m_Pipeline.get());

	vk::RenderPassBeginInfo rpInfo;
	rpInfo.renderPass = state.m_RenderPass->m_RenderPass.get();

	vk::ClearValue clearVal[2];
	clearVal[0].color = vk::ClearColorValue(std::array<float, 4>{ 0.0f, 1.0f, 0.5f, 0.5f });
	clearVal[0].depthStencil.depth = 1;
	clearVal[0].depthStencil.stencil = 0;

	std::fill(std::begin(clearVal) + 1, std::end(clearVal), clearVal[0]);
	rpInfo.clearValueCount = std::size(clearVal);
	rpInfo.pClearValues = clearVal;

	//state.m_RenderPass->m_CreateInfo.

	// Hack!
	const auto& fb = FindOrCreateFramebuffer(*state.m_RenderPass);
	rpInfo.framebuffer = fb.m_Framebuffer.get();

	rpInfo.renderArea.extent.width = fb.m_CreateInfo.width;
	rpInfo.renderArea.extent.height = fb.m_CreateInfo.height;

	buf.beginRenderPass(rpInfo, vk::SubpassContents::eInline);
}

const RenderPass& StateManagerVulkan::FindOrCreateRenderPass(const RenderPassKey& key)
{
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
	std::lock_guard lock(m_Mutex);

	auto& layout = m_StatesToLayouts[key];
	if (!layout)
		layout = CreatePipelineLayout(key);

	return layout;
}

VulkanStateID StateManagerVulkan::FindOrCreateState(LogicalShadowStateID staticID,
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState)
{
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

constexpr PipelineKey::PipelineKey(
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState) :

	RenderPassKey(staticState, dynamicState),
	PipelineLayoutKey(staticState, dynamicState),

	m_DepthCompareFunc(staticState.m_DepthCompareFunc),
	m_DepthTest(staticState.m_DepthTest),
	m_DepthWrite(staticState.m_DepthWrite),

	m_RSBackFaceCulling(staticState.m_RSBackFaceCulling),
	m_RSPolyMode(staticState.m_RSFrontFacePolyMode),

	m_OMSrcFactor(staticState.m_OMSrcFactor),
	m_OMDstFactor(staticState.m_OMDstFactor),

	m_Viewports(dynamicState.m_Viewports)
{
	assert((staticState.m_RSFrontFacePolyMode == staticState.m_RSBackFacePolyMode)
		|| staticState.m_RSBackFaceCulling);

	if (staticState.m_OMDepthRT < 0)
	{
		// Normalize all these so they don't affect the hash
		m_DepthCompareFunc = SHADER_DEPTHFUNC_ALWAYS;
		m_DepthTest = false;
		m_DepthWrite = false;
	}
}

constexpr PipelineLayoutKey::PipelineLayoutKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_VSName(staticState.m_VSName),
	m_VSStaticIndex(staticState.m_VSStaticIndex),
	m_VSVertexFormat(staticState.m_VSVertexFormat),

	m_PSName(staticState.m_PSName),
	m_PSStaticIndex(staticState.m_PSStaticIndex)
{
}

constexpr RenderPassKey::RenderPassKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_OMColorRTs{ staticState.m_OMColorRTs[0], staticState.m_OMColorRTs[1], staticState.m_OMColorRTs[2], staticState.m_OMColorRTs[3] },
	m_OMDepthRT(staticState.m_OMDepthRT)
{
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
}
