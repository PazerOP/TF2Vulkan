#include "interface/internal/IShaderAPIInternal.h"
#include "IStateManagerVulkan.h"
#include "LogicalState.h"
#include "MaterialSystemHardwareConfig.h"
#include "ShaderDevice.h"
#include "shaders/VulkanShaderManager.h"

#include <stdshader_dx9_tf2vulkan/ShaderShared.h>

#include <unordered_map>

using namespace TF2Vulkan;

namespace
{
	struct PipelineKey final
	{
		constexpr PipelineKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_WEAK_EQUALITY_OPERATOR(PipelineKey);

		CUtlSymbolDbg m_VSName;
		int m_VSStaticIndex;
		VertexFormat m_VSVertexFormat;

		CUtlSymbolDbg m_PSName;
		int m_PSStaticIndex;

		bool m_RSBackFaceCulling;
		ShaderPolyMode_t m_RSPolyMode;

		ShaderAPITextureHandle_t m_OMDepthRT;
		ShaderAPITextureHandle_t m_OMColorRTs[4];

		Util::InPlaceVector<ShaderViewport_t, 4> m_Viewports;
	};
}

STD_HASH_DEFINITION(PipelineKey,
	v.m_VSName,
	v.m_VSStaticIndex,
	v.m_VSVertexFormat,

	v.m_PSName,
	v.m_PSStaticIndex,

	v.m_RSBackFaceCulling,
	v.m_RSPolyMode,

	v.m_OMDepthRT,
	v.m_OMColorRTs,

	v.m_Viewports
);

namespace
{
	struct PipelineLayoutKey final
	{
		constexpr PipelineLayoutKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_WEAK_EQUALITY_OPERATOR(PipelineLayoutKey);
	};
}

STD_HASH_DEFINITION(PipelineLayoutKey,

);

namespace
{
	struct RenderPassKey final
	{
		constexpr RenderPassKey(const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);
		DEFAULT_WEAK_EQUALITY_OPERATOR(RenderPassKey);

		ShaderAPITextureHandle_t m_OMDepthRT;
		ShaderAPITextureHandle_t m_OMColorRTs[4];
	};
}

STD_HASH_DEFINITION(RenderPassKey,
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
		std::vector<vk::AttachmentReference> m_Attachments;

		vk::SubpassDescription m_Subpass;
	};

	struct RenderPass final
	{
		std::vector<vk::AttachmentDescription> m_Attachments;
		std::vector<vk::SubpassDependency> m_Dependencies;
		std::vector<Subpass> m_Subpasses;

		vk::RenderPassCreateInfo m_CreateInfo;
		vk::UniqueRenderPass m_RenderPass;

		bool operator!() const { return !m_RenderPass; }
	};

	struct Pipeline final
	{
		std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStageCIs;

		std::vector<vk::VertexInputAttributeDescription> m_VertexInputAttributeDescriptions;
		std::vector<vk::VertexInputBindingDescription> m_VertexInputBindingDescriptions;
		vk::PipelineVertexInputStateCreateInfo m_VertexInputStateCI;

		vk::PipelineInputAssemblyStateCreateInfo m_InputAssemblyStateCI;

		std::vector<vk::Viewport> m_Viewports;
		vk::Rect2D m_Scissor;
		vk::PipelineViewportStateCreateInfo m_ViewportStateCI;

		vk::PipelineRasterizationStateCreateInfo m_RasterizationStateCI;

		vk::PipelineColorBlendStateCreateInfo m_ColorBlendStateCI;

		vk::PipelineMultisampleStateCreateInfo m_MultisampleStateCI;

		const PipelineLayout* m_Layout = nullptr;
		const RenderPass* m_RenderPass = nullptr;

		vk::GraphicsPipelineCreateInfo m_CreateInfo;
		vk::UniquePipeline m_Pipeline;

		bool operator!() const { return !m_Pipeline; }
		VulkanStateID m_ID;
	};
}

namespace
{
	class StateManagerVulkan final : public IStateManagerVulkan
	{
	public:
		void ApplyState(VulkanStateID stateID) override { NOT_IMPLEMENTED_FUNC(); }

		VulkanStateID FindOrCreateState(
			LogicalShadowStateID staticID, const LogicalShadowState& staticState,
			const LogicalDynamicState& dynamicState);

	private:
		const PipelineLayout& FindOrCreatePipelineLayout(
			const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);

		const RenderPass& FindOrCreateRenderPass(
			const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState);

		std::unordered_map<PipelineKey, Pipeline> m_StatesToPipelines;
		std::vector<const Pipeline*> m_IDsToPipelines;
		std::unordered_map<PipelineLayoutKey, PipelineLayout> m_StatesToLayouts;
		std::unordered_map<RenderPassKey, RenderPass> m_StatesToRenderPasses;
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

static vk::PipelineShaderStageCreateInfo CreateStageInfo(
	const CUtlSymbolDbg& name, int staticIndex, vk::ShaderStageFlagBits type)
{
	vk::PipelineShaderStageCreateInfo retVal;
	retVal.stage = type;

	retVal.module = g_ShaderManager.FindOrCreateShader(
		name, staticIndex);

	retVal.pName = "main"; // Shader entry point

	return retVal;
}

static DescriptorSetLayout CreateDescriptorSetLayout(const PipelineLayoutKey& key)
{
	DescriptorSetLayout retVal;

	// Bindings
	{
		auto& bindings = retVal.m_Bindings;
		// BaseTexture
		{
			auto& baseTex = bindings.emplace_back();
			baseTex.binding = 0;
			baseTex.descriptorCount = 1;
			baseTex.descriptorType = vk::DescriptorType::eSampledImage;
			baseTex.stageFlags = vk::ShaderStageFlagBits::eFragment;
		}

		// BaseTextureSampler
		{
			auto& baseTexSampler = bindings.emplace_back();
			baseTexSampler.binding = 1;
			baseTexSampler.descriptorCount = 1;
			baseTexSampler.descriptorType = vk::DescriptorType::eSampler;
			baseTexSampler.stageFlags = vk::ShaderStageFlagBits::eFragment;
		}
	}

	// Descriptor set layout
	{
		auto& ci = retVal.m_CreateInfo;
		ci.pBindings = retVal.m_Bindings.data();
		Util::SafeConvert(retVal.m_Bindings.size(), ci.bindingCount);

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
		g_ShaderDevice.SetDebugName(retVal.m_Layout, "test pipeline layout");
	}

	return retVal;
}

static RenderPass CreateRenderPass(const RenderPassKey& key)
{
	RenderPass retVal;

	// Subpass 0
	{
		auto& sp = retVal.m_Subpasses.emplace_back();

		for (auto& colorID : key.m_OMColorRTs)
		{
			if (colorID < 0)
				continue;

			auto& att = retVal.m_Attachments.emplace_back();
			att.initialLayout = vk::ImageLayout::eUndefined;
			att.finalLayout = vk::ImageLayout::ePresentSrcKHR;
			att.samples = vk::SampleCountFlagBits::e1;
			att.loadOp = vk::AttachmentLoadOp::eDontCare;
			att.storeOp = vk::AttachmentStoreOp::eDontCare;
			att.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			att.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

			auto& attRef = sp.m_Attachments.emplace_back();
			attRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
			Util::SafeConvert(retVal.m_Attachments.size() - 1, attRef.attachment);
		}

		if (key.m_OMDepthRT >= 0)
		{
			auto& att = retVal.m_Attachments.emplace_back();

			auto& attRef = sp.m_Attachments.emplace_back();
			attRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			Util::SafeConvert(retVal.m_Attachments.size() - 1, attRef.attachment);
		}
	}

	// Render pass
	{
		auto& ci = retVal.m_CreateInfo;

		AttachVector(ci.pAttachments, ci.attachmentCount, retVal.m_Attachments);
		AttachVector(ci.pDependencies, ci.dependencyCount, retVal.m_Dependencies);

		std::vector<vk::SubpassDescription> subpassTemp;
		for (auto& sp : retVal.m_Subpasses)
			subpassTemp.push_back(sp.m_Subpass);

		AttachVector(ci.pSubpasses, ci.subpassCount, subpassTemp);

		retVal.m_RenderPass = g_ShaderDevice.GetVulkanDevice().createRenderPassUnique(ci);
		g_ShaderDevice.SetDebugName(retVal.m_RenderPass, "test render pass");
	}

	return retVal;
}

static Pipeline CreatePipeline(const PipelineKey& key,
	const PipelineLayout& layout, const RenderPass& renderPass)
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
			attrs.emplace_back(VIAD(i, 1, vk::Format::eR8G8B8A8Unorm));
		}

		auto & binds = retVal.m_VertexInputBindingDescriptions;
		binds.emplace_back(vk::VertexInputBindingDescription(0, totalVertexSize));

		// "Fake" binding with no real data
		binds.emplace_back(vk::VertexInputBindingDescription(1, sizeof(float) * 4, vk::VertexInputRate::eInstance));

		auto& ci = retVal.m_VertexInputStateCI;
		ci.pVertexBindingDescriptions = binds.data();
		Util::SafeConvert(binds.size(), ci.vertexBindingDescriptionCount);

		ci.pVertexAttributeDescriptions = attrs.data();
		Util::SafeConvert(attrs.size(), ci.vertexAttributeDescriptionCount);
	}

	// Vertex input assembly state create info
	{
		auto& ci = retVal.m_InputAssemblyStateCI;
		ci.topology = vk::PrimitiveTopology::eTriangleList;
	}

	// Viewport/scissor state
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

		Util::SafeConvert(key.m_Viewports.size(), ci.viewportCount);
		ci.pViewports = retVal.m_Viewports.data();

		auto& scissor = retVal.m_Scissor;
		g_ShaderAPIInternal.GetTextureSize(key.m_OMColorRTs[0], scissor.extent.width, scissor.extent.height);

		ci.pScissors = &scissor;
		ci.scissorCount = 1;
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
	}

	// Graphics pipeline
	{
		vk::GraphicsPipelineCreateInfo& ci = retVal.m_CreateInfo;

		Util::SafeConvert(retVal.m_ShaderStageCIs.size(), ci.stageCount);
		ci.pStages = retVal.m_ShaderStageCIs.data();
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
		g_ShaderDevice.SetDebugName(retVal.m_Pipeline, "Test pipeline");
	}

	return retVal;
}

const RenderPass& StateManagerVulkan::FindOrCreateRenderPass(
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState)
{
	const RenderPassKey key(staticState, dynamicState);
	auto& renderPass = m_StatesToRenderPasses[key];
	if (!renderPass)
		renderPass = CreateRenderPass(key);

	return renderPass;
}

const PipelineLayout& StateManagerVulkan::FindOrCreatePipelineLayout(
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState)
{
	const PipelineLayoutKey key(staticState, dynamicState);
	auto& layout = m_StatesToLayouts[key];
	if (!layout)
		layout = CreatePipelineLayout(key);

	return layout;
}

VulkanStateID StateManagerVulkan::FindOrCreateState(LogicalShadowStateID staticID,
	const LogicalShadowState& staticState, const LogicalDynamicState& dynamicState)
{
	const PipelineKey key(staticState, dynamicState);
	auto& pl = m_StatesToPipelines[key];
	if (!pl)
	{
		pl = CreatePipeline(key,
			FindOrCreatePipelineLayout(staticState, dynamicState),
			FindOrCreateRenderPass(staticState, dynamicState));

		m_IDsToPipelines.push_back(&pl);
		assert(VulkanStateID(m_IDsToPipelines.size() - 1) == pl.m_ID);
	}

	return pl.m_ID;
}

constexpr PipelineKey::PipelineKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_VSName(staticState.m_VSName),
	m_VSStaticIndex(staticState.m_VSStaticIndex),
	m_VSVertexFormat(staticState.m_VSVertexFormat),

	m_PSName(staticState.m_PSName),
	m_PSStaticIndex(staticState.m_PSStaticIndex),

	m_RSBackFaceCulling(staticState.m_RSBackFaceCulling),
	m_RSPolyMode(staticState.m_RSFrontFacePolyMode),

	m_OMColorRTs{ staticState.m_OMColorRTs[0], staticState.m_OMColorRTs[1], staticState.m_OMColorRTs[2], staticState.m_OMColorRTs[3] },
	m_OMDepthRT(staticState.m_OMDepthRT),

	m_Viewports(dynamicState.m_Viewports)
{
	assert((staticState.m_RSFrontFacePolyMode == staticState.m_RSBackFacePolyMode)
		|| staticState.m_RSBackFaceCulling);
}

constexpr PipelineLayoutKey::PipelineLayoutKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState)
{
}

constexpr RenderPassKey::RenderPassKey(const LogicalShadowState& staticState,
	const LogicalDynamicState& dynamicState) :

	m_OMColorRTs{ staticState.m_OMColorRTs[0], staticState.m_OMColorRTs[1], staticState.m_OMColorRTs[2], staticState.m_OMColorRTs[3] },
	m_OMDepthRT(staticState.m_OMDepthRT)
{
}
