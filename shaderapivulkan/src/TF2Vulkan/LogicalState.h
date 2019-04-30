#pragma once

#include "VertexFormat.h"

#include <TF2Vulkan/Util/InPlaceVector.h>
#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishaderdynamic.h>
#include <shaderapi/ishadershadow.h>

namespace TF2Vulkan
{
	enum class LogicalShadowStateID : size_t
	{
		Invalid = size_t(-1)
	};

	struct LogicalDynamicState final
	{
		constexpr LogicalDynamicState() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(LogicalDynamicState); // weak because we have floats

		IMaterial* m_BoundMaterial = nullptr;
		int m_BoneCount = 0;
		ShaderAPITextureHandle_t m_FullScreenTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
		uint_fast8_t m_AnisotropicLevel = 0;
		Util::InPlaceVector<ShaderViewport_t, 4> m_Viewports;
		float m_ClearColor[4] = {};

		// Shader constants
		float m_SCOverbright = 1.0f; // standard vertex shader constant
	};

	struct LogicalShadowState final
	{
		constexpr LogicalShadowState() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(LogicalShadowState); // weak because we have floats

		struct Sampler
		{
			constexpr Sampler() = default;
			DEFAULT_STRONG_EQUALITY_OPERATOR(Sampler);

			bool m_Enabled = false;
			bool m_SRGBRead = false;
		};

		// Vertex shader settings
		CUtlSymbolDbg m_VSName;
		int m_VSStaticIndex = 0;
		VertexFormat m_VSVertexFormat;
		MorphFormat_t m_VSMorphFormat = {};

		// Pixel shader settings
		CUtlSymbolDbg m_PSName;
		int m_PSStaticIndex = 0;
		Sampler m_PSSamplers[16];

		// Depth settings
		ShaderDepthFunc_t m_DepthCompareFunc = SHADER_DEPTHFUNC_NEARER;
		bool m_DepthTest = true;
		bool m_DepthWrite = true;

		// Stencil settings
		ShaderStencilFunc_t m_StencilFunc = SHADER_STENCILFUNC_ALWAYS;
		ShaderStencilOp_t m_StencilPassOp = SHADER_STENCILOP_KEEP;
		ShaderStencilOp_t m_StencilFailOp = SHADER_STENCILOP_KEEP;
		ShaderStencilOp_t m_StencilDepthFailOp = SHADER_STENCILOP_KEEP;

		// Rasterizer settings
		bool m_RSBackFaceCulling = true;
		PolygonOffsetMode_t m_RSPolyOffsetMode = SHADER_POLYOFFSET_DISABLE;
		ShaderPolyMode_t m_RSFrontFacePolyMode = SHADER_POLYMODE_FILL;
		ShaderPolyMode_t m_RSBackFacePolyMode = SHADER_POLYMODE_FILL;

		// Output merger settings
		bool m_OMSRGBWrite = false;
		bool m_OMColorWrite = true;
		bool m_OMAlphaTest = false;
		ShaderAlphaFunc_t m_OMAlphaTestFunc = SHADER_ALPHAFUNC_GEQUAL;
		float m_OMAlphaTestRef = 0.5f;
		bool m_OMAlphaBlending = false;
		bool m_OMAlphaWrite = false;
		ShaderBlendFactor_t m_OMSrcFactor = SHADER_BLEND_ZERO;
		ShaderBlendFactor_t m_OMDstFactor = SHADER_BLEND_ONE;
		ShaderAPITextureHandle_t m_OMDepthRT = INVALID_SHADERAPI_TEXTURE_HANDLE;
		ShaderAPITextureHandle_t m_OMColorRTs[4] =
		{
			INVALID_SHADERAPI_TEXTURE_HANDLE,
			INVALID_SHADERAPI_TEXTURE_HANDLE,
			INVALID_SHADERAPI_TEXTURE_HANDLE,
			INVALID_SHADERAPI_TEXTURE_HANDLE
		};

		// Fog settings
		ShaderFogMode_t m_FogMode = SHADER_FOGMODE_DISABLED;
	};
}

STD_HASH_DEFINITION(ShaderViewport_t,
	v.m_nVersion,
	v.m_nTopLeftX,
	v.m_nTopLeftY,
	v.m_nWidth,
	v.m_nHeight,
	v.m_flMinZ,
	v.m_flMaxZ
);

STD_HASH_DEFINITION(TF2Vulkan::LogicalDynamicState,
	v.m_BoundMaterial,
	v.m_BoneCount,
	v.m_FullScreenTexture,
	v.m_AnisotropicLevel,
	v.m_Viewports,
	v.m_ClearColor,
	v.m_SCOverbright
);

STD_HASH_DEFINITION(TF2Vulkan::LogicalShadowState::Sampler,
	v.m_Enabled,
	v.m_SRGBRead
);

STD_HASH_DEFINITION(TF2Vulkan::LogicalShadowState,
	v.m_VSName,
	v.m_VSStaticIndex,
	v.m_VSVertexFormat,
	v.m_VSMorphFormat,

	v.m_PSName,
	v.m_PSStaticIndex,
	v.m_PSSamplers,

	v.m_DepthCompareFunc,
	v.m_DepthTest,
	v.m_DepthWrite,

	v.m_StencilFunc,
	v.m_StencilPassOp,
	v.m_StencilFailOp,
	v.m_StencilDepthFailOp,

	v.m_RSBackFaceCulling,
	v.m_RSPolyOffsetMode,
	v.m_RSFrontFacePolyMode,
	v.m_RSBackFacePolyMode,

	v.m_OMSRGBWrite,
	v.m_OMColorWrite,
	v.m_OMAlphaTest,
	v.m_OMAlphaTestFunc,
	v.m_OMAlphaTestRef,
	v.m_OMAlphaBlending,
	v.m_OMAlphaWrite,
	v.m_OMSrcFactor,
	v.m_OMDstFactor,

	v.m_FogMode
);
