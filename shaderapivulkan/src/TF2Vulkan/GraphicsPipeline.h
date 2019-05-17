#pragma once

#include <TF2Vulkan/VertexFormat.h>

#include <TF2Vulkan/Util/InPlaceVector.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_utility.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <materialsystem/imaterial.h>
#include <shaderapi/ishaderdynamic.h>
#include <shaderapi/ishadershadow.h>
#include <shaderapi/shareddefs.h>

#include <compare>

namespace TF2Vulkan
{
	struct SamplerSettings
	{
		constexpr SamplerSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(SamplerSettings);

		bool m_Enabled = false;
		bool m_SRGBRead = false;
	};

	struct ShaderStageSettings
	{
		constexpr ShaderStageSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(ShaderStageSettings);

		CUtlSymbolDbg m_Name;
		int m_StaticIndex = 0;
	};

	struct VertexShaderStageSettings final : ShaderStageSettings
	{
		constexpr VertexShaderStageSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(VertexShaderStageSettings);

		VertexFormat m_VertexFormat;
		MorphFormat_t m_MorphFormat = {};
	};

	struct PixelShaderStageSettings final : ShaderStageSettings
	{
		constexpr PixelShaderStageSettings() = default;
		bool operator==(const PixelShaderStageSettings& other) const noexcept;

		std::array<SamplerSettings, 16> m_Samplers;
	};

	struct DepthSettings
	{
		constexpr DepthSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(DepthSettings);

		ShaderDepthFunc_t m_CompareFunc = SHADER_DEPTHFUNC_NEARER;
		bool m_DepthTest = true;
		bool m_DepthWrite = true;
	};

	struct StencilSettings
	{
		constexpr StencilSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(StencilSettings);

		ShaderStencilFunc_t m_Func = SHADER_STENCILFUNC_ALWAYS;
		ShaderStencilOp_t m_PassOp = SHADER_STENCILOP_KEEP;
		ShaderStencilOp_t m_FailOp = SHADER_STENCILOP_KEEP;
		ShaderStencilOp_t m_DepthFailOp = SHADER_STENCILOP_KEEP;
	};

	struct DepthStencilSettings
	{
		constexpr DepthStencilSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(DepthStencilSettings);

		DepthSettings m_Depth;
		StencilSettings m_Stencil;
	};

	struct RasterizerFaceSettings
	{
		constexpr RasterizerFaceSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(RasterizerFaceSettings);

		ShaderPolyMode_t m_PolyMode = SHADER_POLYMODE_FILL;
	};

	struct RasterizerSettings
	{
		constexpr RasterizerSettings() = default;
		DEFAULT_STRONG_EQUALITY_OPERATOR(RasterizerSettings);

		bool m_BackfaceCulling = true;
		PolygonOffsetMode_t m_OffsetMode = SHADER_POLYOFFSET_DISABLE;
		RasterizerFaceSettings m_FrontFace;
		RasterizerFaceSettings m_BackFace;
	};

	struct BlendSettings
	{
		constexpr BlendSettings() = default;

		DEFAULT_STRONG_EQUALITY_OPERATOR(BlendSettings);

		bool m_SRGBWrite = false;

		bool m_ColorWrite = true;

		bool m_AlphaTest = false;
		ShaderAlphaFunc_t m_AlphaTestFunc = SHADER_ALPHAFUNC_GEQUAL;
		uint_fast8_t m_AlphaTestRef = 0.5f;

		bool m_AlphaBlending = false;
		bool m_AlphaWrite = false;

		ShaderBlendFactor_t m_SrcFactor = SHADER_BLEND_ZERO;
		ShaderBlendFactor_t m_DstFactor = SHADER_BLEND_ONE;
	};

	struct RenderPassSettings
	{
		constexpr RenderPassSettings() = default;

		DEFAULT_STRONG_EQUALITY_OPERATOR(RenderPassSettings);

		ShaderAPITextureHandle_t m_RenderTargetColors[4] = { 0, -1, -1, -1 };
		ShaderAPITextureHandle_t m_RenderTargetDepth = -1;
	};

	struct PipelineSettings
	{
		constexpr PipelineSettings() = default;

		DEFAULT_STRONG_EQUALITY_OPERATOR(PipelineSettings);

		VertexShaderStageSettings m_VertexShader;
		PixelShaderStageSettings m_PixelShader;

		DepthStencilSettings m_DepthStencil;
		RasterizerSettings m_Rasterizer;
		BlendSettings m_Blend;
		RenderPassSettings m_RenderPass;
	};
}

STD_HASH_DEFINITION(TF2Vulkan::SamplerSettings,
	v.m_Enabled,
	v.m_SRGBRead
);

STD_HASH_DEFINITION(TF2Vulkan::ShaderStageSettings,
	v.m_Name,
	v.m_StaticIndex
);

STD_HASH_DEFINITION(TF2Vulkan::VertexShaderStageSettings,
	*(const TF2Vulkan::ShaderStageSettings*)this,
	v.m_VertexFormat
);

STD_HASH_DEFINITION(TF2Vulkan::PixelShaderStageSettings,
	*(const TF2Vulkan::ShaderStageSettings*)this,
	v.m_Samplers
);

STD_HASH_DEFINITION(TF2Vulkan::DepthSettings,
	v.m_CompareFunc,
	v.m_DepthTest,
	v.m_DepthWrite
);

STD_HASH_DEFINITION(TF2Vulkan::StencilSettings,
	v.m_Func,
	v.m_PassOp,
	v.m_FailOp,
	v.m_DepthFailOp
);

STD_HASH_DEFINITION(TF2Vulkan::DepthStencilSettings,
	v.m_Depth,
	v.m_Stencil
);

STD_HASH_DEFINITION(TF2Vulkan::RasterizerSettings,
	v.m_BackfaceCulling
);

STD_HASH_DEFINITION(TF2Vulkan::BlendSettings,
	v.m_SRGBWrite,
	v.m_ColorWrite,
	v.m_AlphaTest,
	v.m_AlphaBlending,
	v.m_AlphaWrite
);

STD_HASH_DEFINITION(TF2Vulkan::PipelineSettings,
	v.m_VertexShader,
	v.m_PixelShader,

	v.m_DepthStencil,
	v.m_Rasterizer,
	v.m_Blend
);
