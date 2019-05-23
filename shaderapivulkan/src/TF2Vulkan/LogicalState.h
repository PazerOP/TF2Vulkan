#pragma once

#include <stdshader_vulkan/ShaderData.h>

#include <TF2Vulkan/VertexFormat.h>
#include <TF2Vulkan/IBufferPool.h>
#include <TF2Vulkan/Util/InPlaceVector.h>
#include <TF2Vulkan/Util/lightdesc.h>
#include <TF2Vulkan/Util/shaderapi_ishaderdynamic.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/std_compare.h>
#include <TF2Vulkan/Util/std_stack.h>
#include <TF2Vulkan/Util/utlsymbol.h>
#include <TF2Vulkan/Util/vmatrix.h>
#include <TF2Vulkan/Util/vector.h>

#include <renderparm.h>
#include <shaderapi/ishaderapi.h>
#include <shaderapi/ishadershadow.h>

namespace TF2Vulkan
{
	class IShaderGroupInternal;
	class IShaderInstanceInternal;

	enum class LogicalShadowStateID : size_t
	{
		Invalid = size_t(-1)
	};

	struct LogicalDynamicState final
	{
		constexpr LogicalDynamicState() = default;

		bool m_InFlashlightMode = false;
		IMaterial* m_BoundMaterial = nullptr;
		int m_BoneCount = 0;
		ShaderAPITextureHandle_t m_FullScreenTexture = INVALID_SHADERAPI_TEXTURE_HANDLE;
		uint_fast8_t m_AnisotropicLevel = 0;
		Util::InPlaceVector<ShaderViewport_t, 4> m_Viewports;
		std::array<float, 4> m_ClearColor = {};
		MaterialFogMode_t m_SceneFogMode = MATERIAL_FOG_NONE;
		Vector m_WorldSpaceCameraPosition;
		bool m_ForceDepthFuncEquals = false;
		bool m_FBLinear = false;
		Vector m_TonemappingScale{ 1, 1, 1 };

		const IShaderInstanceInternal* m_VSShader = nullptr;
		const IShaderInstanceInternal* m_PSShader = nullptr;

		std::array<BufferPoolEntry, 8> m_UniformBuffers = {};

		// Scissor settings
		bool m_ScissorEnable = false;
		int m_ScissorX = -1;
		int m_ScissorY = -1;
		int m_ScissorWidth = -1;
		int m_ScissorHeight = -1;

		// Stencil settings
		bool m_StencilEnable = false;
		StencilOperation_t m_StencilFailOp = STENCILOPERATION_KEEP;
		StencilOperation_t m_StencilDepthFailOp = STENCILOPERATION_KEEP;
		StencilOperation_t m_StencilPassOp = STENCILOPERATION_KEEP;
		StencilComparisonFunction_t m_StencilCompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		uint32_t m_StencilRef = 0;
		uint32_t m_StencilTestMask = 0xFFFFFFFF;
		uint32_t m_StencilWriteMask = 0xFFFFFFFF;

		MaterialCullMode_t m_CullMode = MATERIAL_CULLMODE_CCW;

		LightState_t m_LightState;
		std::array<LightDesc_t, 4> m_Lights;
		std::array<Vector4D, 6> m_LightAmbientCube;

		std::array<Vector, MAX_VECTOR_RENDER_PARMS> m_RenderParamsVector;
		std::array<int, MAX_INT_RENDER_PARMS> m_RenderParamsInt;
		std::array<float, MAX_FLOAT_RENDER_PARMS> m_RenderParamsFloat;

		std::array<ShaderAPITextureHandle_t, 16> m_BoundTextures{};

		std::array<matrix3x4_t, 53> m_BoneMatrices{};

		bool m_UserClipTransformOverrideEnabled = false;
		VMatrix m_UserClipTransformOverride{};
	};

	struct LogicalShadowState final
	{
		constexpr LogicalShadowState() = default;
		DEFAULT_WEAK_EQUALITY_OPERATOR(LogicalShadowState); // weak because we have floats

		struct Sampler
		{
			constexpr Sampler() = default;
			DEFAULT_STRONG_EQUALITY_OPERATOR(Sampler);

			bool m_SRGBRead = false;
		};

		bool m_FSAlphaToCoverage = false;
		bool m_FSFogGammaCorrection = true;

		// Vertex shader settings
		const IShaderGroupInternal* m_VSShader = nullptr;
		VertexFormat m_VSVertexFormat;
		MorphFormat_t m_VSMorphFormat = {};

		// Pixel shader settings
		const IShaderGroupInternal* m_PSShader = nullptr;
		std::array<Sampler, 16> m_PSSamplers;

		// Depth settings
		ShaderDepthFunc_t m_DepthCompareFunc = SHADER_DEPTHFUNC_NEARER;
		bool m_DepthTest = true;
		bool m_DepthWrite = true;

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
		ShaderAPITextureHandle_t m_OMDepthRT = 0;
		std::array<ShaderAPITextureHandle_t, 4> m_OMColorRTs = { 0, -1, -1, -1 };

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

STD_HASH_DEFINITION(TF2Vulkan::LogicalShadowState::Sampler,
	v.m_SRGBRead
);

STD_HASH_DEFINITION(TF2Vulkan::LogicalShadowState,
	v.m_VSShader,
	v.m_VSVertexFormat,
	v.m_VSMorphFormat,

	v.m_PSShader,
	v.m_PSSamplers,

	v.m_DepthCompareFunc,
	v.m_DepthTest,
	v.m_DepthWrite,

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
	v.m_OMDepthRT,
	v.m_OMColorRTs,

	v.m_FogMode
);
