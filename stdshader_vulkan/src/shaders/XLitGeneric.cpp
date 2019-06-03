#include "BaseShaderNext.h"
#include "ParamGroups.h"

#include "UniformBufConstructs/TextureTransform.h"

#include <stdshader_vulkan/ShaderData.h>
#include <TF2Vulkan/AlignedTypes.h>
#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/ISpecConstLayout.h>
#include <TF2Vulkan/IBufferPool.h>
#include <TF2Vulkan/VertexFormat.h>
#include <TF2Vulkan/Util/std_array.h>
#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <shaderlib/BaseShader.h>
#include <shaderlib/cshader.h>
#include <shaderlib/ShaderDLL.h>

#include "ShaderComponents/Bumpmap.h"
#include "ShaderComponents/Detail.h"
#include "ShaderComponents/Phong.h"
#include "ShaderComponents/Rimlight.h"
#include "ShaderComponents/SelfIllum.h"
#include "ShaderComponents/WeaponSheen.h"

#include <vector>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;
using namespace TF2Vulkan::Shaders::Components;

inline namespace XLitGeneric
{
	static constexpr float1 DEFAULT_CLOAK_FACTOR = 0;
	static constexpr float4 DEFAULT_CLOAK_COLOR_TINT{ 1, 1, 1, 1 };
	static constexpr float1 DEFAULT_REFRACT_AMOUNT = 0.1f;

	struct BaseTextureComponent
	{
		struct SpecConstBuf
		{
			uint1 TEXINDEX_BASETEXTURE;
		};
		template<typename T> struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, TEXINDEX_BASETEXTURE);
		};
		struct UniformBuf
		{
			TextureTransform m_BaseTextureTransform;
		};
		struct Params
		{
		private:
			template<typename... TGroups> friend class TF2Vulkan::Shaders::ShaderComponents;
			void InitParamGroup(IMaterialVar** params) const {}
			void PreDraw(IMaterialVar** params, UniformBuf* uniformBuf, SpecConstBuf* specConstBuf, ShaderTextureBinder& tb) const
			{
				if (params[BASETEXTURE]->IsTexture())
				{
					tb.AddBinding(params[BASETEXTURE]->GetTextureValue(), specConstBuf->TEXINDEX_BASETEXTURE);
					uniformBuf->m_BaseTextureTransform = TextureTransform(params[BASETEXTURETRANSFORM]->GetMatrixValue());
				}
			}
			void LoadResources(IMaterialVar** params, IShaderInit& init, const char* materialName, const char* texGroupName) const
			{
				if (params[BASETEXTURE]->IsDefined())
					init.LoadTexture(params[BASETEXTURE], texGroupName);
			}
		};
	};

	struct XLitGenericComponent
	{
		struct SpecConstBuf
		{
			uint1 TEXTURE_COUNT;
			uint1 SAMPLER_COUNT;

			bool32 VERTEXCOLOR;
			bool32 SKINNING;
			bool32 COMPRESSED_VERTS;
			bool32 DIFFUSELIGHTING;
			bool32 GAMMA_CONVERT_VERTEX_COLOR;

			bool32 AMBIENT_LIGHT;
			bool32 DYNAMIC_LIGHT;
		};
		template<typename T>
		struct SpecConstLayout
		{
			SPEC_CONST_BUF_ENTRY(T, TEXTURE_COUNT);
			SPEC_CONST_BUF_ENTRY(T, SAMPLER_COUNT);

			SPEC_CONST_BUF_ENTRY(T, VERTEXCOLOR);
			SPEC_CONST_BUF_ENTRY(T, SKINNING);
			SPEC_CONST_BUF_ENTRY(T, COMPRESSED_VERTS);
			SPEC_CONST_BUF_ENTRY(T, DIFFUSELIGHTING);
			SPEC_CONST_BUF_ENTRY(T, GAMMA_CONVERT_VERTEX_COLOR);

			SPEC_CONST_BUF_ENTRY(T, AMBIENT_LIGHT);
			SPEC_CONST_BUF_ENTRY(T, DYNAMIC_LIGHT);
		};

		struct UniformBuf
		{
			float1 m_CloakFactor;
			float1 m_RefractAmount;
			float4 m_RefractColorTint;
			float4x2 m_ViewProjR01;

			float4 m_MorphSubrect;
			float3 m_MorphTargetTextureDim;
			float m_SeamlessScale;
			float4x4 m_FlashlightWorldToTexture;

			float m_VertexAlpha;
		};

		struct Params
		{
			//NSHADER_PARAM(ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)");
			NSHADER_PARAM(LIGHTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "lightmap texture--will be bound by the engine");

			// Debugging term for visualizing ambient data on its own
			NSHADER_PARAM(AMBIENTONLY, SHADER_PARAM_TYPE_INTEGER, "0", "Control drawing of non-ambient light ()");

			NSHADER_PARAM(LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term");

			NSHADER_PARAM(LINEARWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of shader results.");
			NSHADER_PARAM(GAMMACOLORREAD, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of color texture read.");

			NSHADER_PARAM(BLENDTINTBYBASEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use the base alpha to blend in the $color modulation");
			NSHADER_PARAM(BLENDTINTCOLOROVERBASE, SHADER_PARAM_TYPE_FLOAT, "0", "blend between tint acting as a multiplication versus a replace");

			NSHADER_PARAM(HDRCOLORSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "hdr color scale");

			NSHADER_PARAM(FLASHLIGHTNOLAMBERT, SHADER_PARAM_TYPE_BOOL, "0", "Flashlight pass sets N.L=1.0");
			NSHADER_PARAM(RECEIVEFLASHLIGHT, SHADER_PARAM_TYPE_INTEGER, "0", "Forces this material to receive flashlights.");

		private:
			template<typename... TGroups> friend class ShaderComponents;
			void InitParamGroup(IMaterialVar** params) const {}
			void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf, ShaderTextureBinder& tb) const {}
			void LoadResources(IMaterialVar** params, IShaderInit& init, const char* materialName, const char* texGroupName) const {}
		};
	};

	using XLitGenericComponents = ShaderComponents<
		//Components::AlphaTest,
		Components::Bumpmap,
		//Components::DepthBlend,
		//Components::Wrinkle,
		//Components::EnvMap,
		//Components::Phong,
		//Components::Rimlight,
		//Components::SelfIllum,
		Components::Detail,
		//Components::EmissiveScroll,
		//Components::WeaponSheen,
		//Components::SeamlessScale,
		//Components::Cloak,
		//Components::Flesh,
		//Components::DistanceAlpha,
		BaseTextureComponent,
		XLitGenericComponent>;

	enum class DerivedShaderType
	{
		VertexLitGeneric,
		UnlitGeneric,
		Wireframe,
		DepthWrite,
		DebugDrawEnvmapMask,
		EyeRefract
	};

	class Shader : public ShaderNext<Shader, XLitGenericComponents>
	{
	public:
		explicit Shader(DerivedShaderType derivedType) : m_DerivedType(derivedType) {}

		void OnInitShader(IShaderNextFactory& instanceMgr) override;
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
			const char* materialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;

		DerivedShaderType GetDerivedType() const { return m_DerivedType; }

		bool IsVertexLit() const { return GetDerivedType() == DerivedShaderType::VertexLitGeneric; }

		virtual bool SupportsCompressedVertices() const { return true; }

	private:
		BlendType_t EvaluateBlendRequirements() const;

		struct DrawParams : OnDrawElementsParams
		{
			DrawParams(const OnDrawElementsParams& base) : OnDrawElementsParams(base) {}

			SpecConstBuf m_SpecConsts;
			VertexFormat m_Format;
			ShaderDataCommon m_UniformsCommon{};
			VSModelMatrices m_ModelMatrices{};
			UniformBuf m_Uniforms;
			ShaderTextureBinder m_TextureBinder;

			bool m_SRGBWrite = true;
		};

		IShaderGroup* m_PSShader = nullptr;
		IShaderGroup* m_VSShader = nullptr;

		IBufferPool* m_UniformBufPool = nullptr;
		UniformBufferIndex m_UBufCommonIndex = UniformBufferIndex::Invalid;
		UniformBufferIndex m_UBufModelMatricesIndex = UniformBufferIndex::Invalid;
		UniformBufferIndex m_UniformBufferIndex = UniformBufferIndex::Invalid;

		DerivedShaderType m_DerivedType;
	};

#define XLITGENERIC_DERIVED(derivedShaderName) \
	class derivedShaderName : public Shader \
	{ \
	public: \
		derivedShaderName() : Shader(DerivedShaderType::derivedShaderName) {} \
		const char* GetName() const override { return #derivedShaderName; } \
	}; \
	static const DefaultInstanceRegister<derivedShaderName> s_ ## derivedShaderName;

#define SHADER_ALIAS(aliasName, baseName) \
	class aliasName final : public baseName \
	{ \
	public: \
		const char* GetName() const override { return #aliasName; } \
	}; \
	static const DefaultInstanceRegister<aliasName> s_ ## aliasName;

	XLITGENERIC_DERIVED(DebugDrawEnvmapMask);
	XLITGENERIC_DERIVED(DepthWrite);
	XLITGENERIC_DERIVED(UnlitGeneric);
	XLITGENERIC_DERIVED(VertexLitGeneric);
	XLITGENERIC_DERIVED(EyeRefract);

	class Wireframe : public Shader
	{
	public:
		Wireframe() : Shader(DerivedShaderType::Wireframe) {}
		const char* GetName() const override { return "Wireframe"; }
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;
		bool SupportsCompressedVertices() const override { return false; }
	};
	static const DefaultInstanceRegister<Wireframe> s_Wireframe;

	SHADER_ALIAS(Wireframe_DX8, Wireframe);
	SHADER_ALIAS(Wireframe_DX9, Wireframe);
}

// FIXME: Implement Shadow shader (it can't really derive from XLitGeneric)
DEFINE_NSHADER_FALLBACK(Shadow, UnlitGeneric);

DEFINE_NSHADER_FALLBACK(UnlitGeneric_DX8, UnlitGeneric);
DEFINE_NSHADER_FALLBACK(DebugMorphAccumulator, UnlitGeneric);
//DEFINE_NSHADER_FALLBACK(VertexLitGeneric_DX8, VertexLitGeneric);

static ConVar mat_phong("mat_phong", "1");
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar r_lightwarpidentity("r_lightwarpidentity", "0", FCVAR_CHEAT);
static ConVar mat_luxels("mat_luxels", "0", FCVAR_CHEAT);

void Wireframe::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	LOG_FUNC();

	SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	SET_FLAGS(MATERIAL_VAR_NOFOG);
	SET_FLAGS(MATERIAL_VAR_WIREFRAME);

	Shader::OnInitShaderParams(params, materialName);
}

void Wireframe::OnDrawElements(const OnDrawElementsParams& params)
{
	LOG_FUNC();

	//if (IsSnapshotting())
	//	__debugbreak();

	Shader::OnDrawElements(params);
}

void Shader::OnInitShader(IShaderNextFactory& instanceMgr)
{
	LOG_FUNC();

	m_VSShader = &instanceMgr.FindOrCreateShaderGroup(ShaderType::Vertex, "xlitgeneric_vs", s_SpecConstLayout);
	m_PSShader = &instanceMgr.FindOrCreateShaderGroup(ShaderType::Pixel, "xlitgeneric_ps", s_SpecConstLayout);

	m_UniformBufPool = &instanceMgr.GetUniformBufferPool();
	m_UBufCommonIndex = m_VSShader->FindUniformBuffer(UniformBufferStandardType::ShaderCommon);
	m_UBufModelMatricesIndex = m_VSShader->FindUniformBuffer(UniformBufferStandardType::VSModelMatrices);
	m_UniformBufferIndex = m_VSShader->FindUniformBuffer(UniformBufferStandardType::ShaderCustom);
}

void Shader::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	LOG_FUNC();

	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	if (g_pHardwareConfig->SupportsBorderColor())
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
	}

	// This shader can be used with hw skinning
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	if (IsVertexLit())
	{
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
	}
	else
	{
		CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
	}

	// TODO: These don't match the ShaderParamNext default for SEAMLESS_SCALE
	//InitFloatParam(SEAMLESS_SCALE, params, 0.0);
	//InitFloatParam(EDGESOFTNESSSTART, params, 0.5);
	//InitFloatParam(OUTLINEALPHA, params, 1.0);

	// No texture means no self-illum or env mask in base alpha
	if (!params[BASETEXTURE]->IsDefined())
	{
		CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	}

	if ((g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined())
		// we don't need a tangent space if we have envmap without bumpmap
		//		|| ( info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() )
		)
	{
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}
	else if (params[LIGHTWARPTEXTURE]->IsDefined()) // diffuse warp goes down bump path...
	{
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}
	else // no tangent space needed
	{
		CLEAR_FLAGS(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
	}

	const bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);

#if false
	if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) && params[BUMPMAP]->IsDefined() && !hasNormalMapAlphaEnvmapMask)
	{
		Warning("material %s has a normal map and $basealphaenvmapmask.  Must use $normalmapalphaenvmapmask to get specular.\n\n", materialName);
		params[ENVMAP]->SetUndefined();
	}

	if (params[ENVMAPMASK]->IsDefined() && params[BUMPMAP]->IsDefined())
	{
		params[ENVMAPMASK]->SetUndefined();
		if (!hasNormalMapAlphaEnvmapMask)
		{
			Warning("material %s has a normal map and an envmapmask.  Must use $normalmapalphaenvmapmask.\n\n", materialName);
			params[ENVMAP]->SetUndefined();
		}
	}

	// If mat_specular 0, then get rid of envmap
	if (!g_pConfig->UseSpecular() && params[ENVMAP]->IsDefined() && params[BASETEXTURE]->IsDefined())
		params[ENVMAP]->SetUndefined();
#endif
}

void Shader::OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
	const char* materialName)
{
}

BlendType_t Shader::EvaluateBlendRequirements() const
{
	LOG_FUNC();

	bool isTranslucent = IsAlphaModulating();

	const auto& textureVar = BASETEXTURE;
	const bool isBaseTexture = true;
	const auto& detailTextureVar = DETAIL;

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha (for blending or alpha test)
	isTranslucent = isTranslucent || (TextureIsTranslucent(textureVar, isBaseTexture) &&
		!(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST));

	if ((detailTextureVar != -1) && (!isTranslucent))
	{
		isTranslucent = TextureIsTranslucent(detailTextureVar, isBaseTexture);
	}

	if (CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE)
	{
		return isTranslucent ? BT_BLENDADD : BT_ADD;	// Additive
	}
	else
	{
		return isTranslucent ? BT_BLEND : BT_NONE;		// Normal blending
	}
}

void Shader::OnDrawElements(const OnDrawElementsParams& params)
{
	LOG_FUNC();

	DrawParams drawParams(params);

	const bool bVertexLitGeneric = IsVertexLit();
	const bool hasDiffuseLighting = drawParams.m_SpecConsts.DIFFUSELIGHTING = bVertexLitGeneric;
	const bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
	//const bool bHasBaseTexture = drawParams.m_SpecConsts.TEXACTIVE_BASETEXTURE = params[BASETEXTURE]->IsTexture();
	const bool bIsAdditive = CShader_IsFlagSet(params.matvars, MATERIAL_VAR_ADDITIVE);
	const bool bHasFlashlight = false;
	//const bool bHasBump = drawParams.m_SpecConsts.TEXACTIVE_BUMPMAP = (g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsTexture());
	//drawParams.m_SpecConsts.NORMALMAPPING = bHasBump;

	const bool bHasVertexColor = !bVertexLitGeneric && IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
	const bool bHasVertexAlpha = !bVertexLitGeneric && IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);
	drawParams.m_SpecConsts.VERTEXCOLOR = bHasVertexColor || bHasVertexAlpha;

	//const bool bSeamlessBase = params[SEAMLESS_BASE]->GetBoolValue();
	//const bool bSeamlessDetail = params[SEAMLESS_DETAIL]->GetBoolValue();
	//const bool bHasEnvmap = !bHasFlashlight && params[ENVMAP]->IsTexture();
	const bool bHasNormal = IsPC();// || bVertexLitGeneric || bHasEnvmap || bHasFlashlight || bSeamlessBase || bSeamlessDetail;

	const bool bSRGBWrite = !params[LINEARWRITE]->GetBoolValue();
	drawParams.m_SpecConsts.GAMMA_CONVERT_VERTEX_COLOR = !(!bSRGBWrite && bHasVertexColor);

	//static constexpr auto SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
	//static constexpr auto SAMPLER_BUMPMAP = SHADER_SAMPLER1;
	//static constexpr auto SAMPLER_REFRACT = SHADER_SAMPLER2;

	if (const auto shadow = params.shadow)
	{
		shadow->SetShaders(m_VSShader, m_PSShader);

		// Alpha blending
		SetBlendingShadowState(EvaluateBlendRequirements());

		drawParams.m_Format.AddFlags(VertexFormatFlags::Position);

		if (SupportsCompressedVertices())
			drawParams.m_Format.AddFlags(VertexFormatFlags::Meta_Compressed);

		if (drawParams.m_SpecConsts.VERTEXCOLOR)
			drawParams.m_Format.AddFlags(VertexFormatFlags::Color);

		if (bHasNormal)
			drawParams.m_Format.AddFlags(VertexFormatFlags::Normal);

		if (params[BASETEXTURE]->IsTexture())
			drawParams.m_Format.AddTexCoord();

		{
			int texCoordSizes[8];
			drawParams.m_Format.GetTexCoordSizes(texCoordSizes);
			params.shadow->VertexShaderVertexFormat((unsigned int)drawParams.m_Format.m_Flags,
				drawParams.m_Format.GetTexCoordCount(), texCoordSizes, drawParams.m_Format.m_UserDataSize);
		}

		//if (bHasBaseTexture)
		//	shadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);

		//if (drawParams.m_UsingRefraction)
		//	shadow->EnableSRGBRead(SAMPLER_REFRACT, true);

		//if (bHasBump)
		//	shadow->EnableSRGBRead(SAMPLER_BUMPMAP, false);

		shadow->EnableSRGBWrite(bSRGBWrite);
	}

	if (const auto dynamic = params.dynamic)
	{
		PreDraw(params.matvars, drawParams.m_Uniforms, drawParams.m_SpecConsts, drawParams.m_TextureBinder);

		drawParams.m_SpecConsts.COMPRESSED_VERTS = params.compression;
		drawParams.m_SpecConsts.SKINNING = dynamic->GetCurrentNumBones() > 0;

		auto& common = drawParams.m_UniformsCommon;
		auto& modelMats = drawParams.m_ModelMatrices;
		auto& custom = drawParams.m_Uniforms;
		dynamic->GetWorldSpaceCameraPosition(common.m_EyePos);

		if (bVertexLitGeneric)
		{
			drawParams.m_SpecConsts.DYNAMIC_LIGHT = true;
			LoadLights(common);

			drawParams.m_SpecConsts.AMBIENT_LIGHT = (common.m_AmbientCube != AmbientLightCube());
		}

		custom.m_VertexAlpha = bHasVertexAlpha ? 1 : 0;

		// Base Matrices
		{
			VMatrix model, view, proj;
			dynamic->GetMatrix(MATERIAL_MODEL, model);
			dynamic->GetMatrix(MATERIAL_VIEW, view);
			dynamic->GetMatrix(MATERIAL_PROJECTION, proj);

			common.m_ViewProj = view * proj;
			common.m_ModelViewProj = (model * common.m_ViewProj).Transpose();
			common.m_ViewProj = common.m_ViewProj.Transpose();
		}

		// Model matrices
		dynamic->LoadBoneMatrices(modelMats);

		//if (drawParams.m_UsingRefraction)
		//	dynamic->BindStandardTexture(SAMPLER_REFRACT, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

		// Bind textures
		{
			uint32_t& texCount = drawParams.m_SpecConsts.TEXTURE_COUNT;
			texCount = 1;
			for (auto& tex : drawParams.m_TextureBinder)
			{
				BindTexture(Sampler_t(SHADER_SAMPLER0 + texCount), tex.m_Texture);
				texCount++;
			}
		}

		// Update data and bind uniform buffers
		params.dynamic->BindUniformBuffer(m_UniformBufPool->Create(drawParams.m_Uniforms), m_UniformBufferIndex);
		params.dynamic->BindUniformBuffer(m_UniformBufPool->Create(drawParams.m_UniformsCommon), m_UBufCommonIndex);
		params.dynamic->BindUniformBuffer(m_UniformBufPool->Create(drawParams.m_ModelMatrices), m_UBufModelMatricesIndex);

		// Set
		params.dynamic->SetVertexShader(m_VSShader->FindOrCreateInstance(drawParams.m_SpecConsts));
		params.dynamic->SetPixelShader(m_PSShader->FindOrCreateInstance(drawParams.m_SpecConsts));
	}

	Draw();
}
