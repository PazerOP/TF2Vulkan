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

#include <vector>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

inline namespace XLitGeneric
{
	static constexpr float1 DEFAULT_CLOAK_FACTOR = 0;
	static constexpr float4 DEFAULT_CLOAK_COLOR_TINT{ 1, 1, 1, 1 };
	static constexpr float1 DEFAULT_REFRACT_AMOUNT = 0.1f;

	struct Params : BumpmapParams, WrinkleParams, EnvMapParams, PhongParams, RimlightParams, SelfillumParams, DetailParams, EmissiveScrollParams, WeaponSheenParams, SeamlessScaleParams, CloakParams, FleshParams, DistanceAlphaParams
	{
		NSHADER_PARAM(ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)");
		NSHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "");
		NSHADER_PARAM(FLASHLIGHTNOLAMBERT, SHADER_PARAM_TYPE_BOOL, "0", "Flashlight pass sets N.L=1.0");
		NSHADER_PARAM(LIGHTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "lightmap texture--will be bound by the engine");

		// Debugging term for visualizing ambient data on its own
		NSHADER_PARAM(AMBIENTONLY, SHADER_PARAM_TYPE_INTEGER, "0", "Control drawing of non-ambient light ()");

		NSHADER_PARAM(LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term");
		NSHADER_PARAM(ENVMAPFRESNEL, SHADER_PARAM_TYPE_FLOAT, "0", "Degree to which Fresnel should be applied to env map");

		NSHADER_PARAM(SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture");
		NSHADER_PARAM(LINEARWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of shader results.");
		NSHADER_PARAM(DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries. Only supported without bumpmaps");
		NSHADER_PARAM(DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges.");

		NSHADER_PARAM(BLENDTINTBYBASEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use the base alpha to blend in the $color modulation");
		NSHADER_PARAM(BLENDTINTCOLOROVERBASE, SHADER_PARAM_TYPE_FLOAT, "0", "blend between tint acting as a multiplication versus a replace");

		NSHADER_PARAM(VERTEXALPHATEST, SHADER_PARAM_TYPE_INTEGER, "0", "");
		NSHADER_PARAM(HDRCOLORSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "hdr color scale");
		NSHADER_PARAM(RECEIVEFLASHLIGHT, SHADER_PARAM_TYPE_INTEGER, "0", "Forces this material to receive flashlights.");
		NSHADER_PARAM(GAMMACOLORREAD, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of color texture read.");
	};

	struct UniformBuf final
	{
		TextureTransform m_BaseTextureTransform;
		TextureTransform m_DetailTransform;
		TextureTransform m_BumpTransform;

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

	struct SpecConstBuf final : BaseSpecConstBuffer<SpecConstBuf>
	{
		bool32 VERTEXCOLOR;
		bool32 SKINNING;
		bool32 COMPRESSED_VERTS;
		bool32 DIFFUSELIGHTING;
		bool32 GAMMA_CONVERT_VERTEX_COLOR;

		bool32 DYNAMIC_LIGHT;

		bool32 TEXACTIVE_BASETEXTURE;

		bool32 TEXACTIVE_BUMPMAP;
		bool32 NORMALMAPPING;
	};

	struct SpecConstLayout final : BaseSpecConstLayout<SpecConstLayout, SpecConstBuf>
	{
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, VERTEXCOLOR);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, SKINNING);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, COMPRESSED_VERTS);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, DIFFUSELIGHTING);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, GAMMA_CONVERT_VERTEX_COLOR);

		SPEC_CONST_BUF_ENTRY(SpecConstBuf, DYNAMIC_LIGHT);

		SPEC_CONST_BUF_ENTRY(SpecConstBuf, TEXACTIVE_BASETEXTURE);

		SPEC_CONST_BUF_ENTRY(SpecConstBuf, TEXACTIVE_BUMPMAP);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, NORMALMAPPING);

	} static constexpr s_SpecConstLayout;

	enum class DerivedShaderType
	{
		VertexLitGeneric,
		UnlitGeneric,
		Wireframe,
		DepthWrite,
		DebugDrawEnvmapMask,
		EyeRefract
	};

	class Shader : public ShaderNext<Shader, Params>
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
		bool WantsSkinShader(IMaterialVar** params) const;
		bool WantsPhongShader(IMaterialVar** params) const;

		void InitParamsVertexLitGeneric(IMaterialVar** params, const char* materialName) const;
		void InitParamsSkin(IMaterialVar** params) const;
		void InitParamsCloakBlendedPass(IMaterialVar** params) const;
		void InitParamsWeaponSheenPass(IMaterialVar** params) const;
		void InitParamsEmissiveScrollBlendedPass(IMaterialVar** params) const;
		void InitParamsFleshInteriorBlendedPass(IMaterialVar** params) const;

		void InitShaderVertexLitGeneric(IMaterialVar** params);
		void InitShaderSkin(IMaterialVar** params);
		void InitShaderCloakBlendedPass(IMaterialVar** params);
		void InitShaderWeaponSheenPass(IMaterialVar** params);
		void InitShaderEmissiveScrollBlendedPass(IMaterialVar** params);
		void InitShaderFleshInteriorBlendedPass(IMaterialVar** params);

		BlendType_t EvaluateBlendRequirements() const;

		struct DrawParams : OnDrawElementsParams
		{
			DrawParams(const OnDrawElementsParams& base) : OnDrawElementsParams(base) {}

			SpecConstBuf m_SpecConsts;
			VertexFormat m_Format;
			ShaderDataCommon m_UniformsCommon{};
			VSModelMatrices m_ModelMatrices{};
			UniformBuf m_Uniforms{};

			bool m_UsingBaseTexture = false;
			bool m_UsingRefraction = false;
			bool m_SRGBWrite = true;
		};

		void SetupPhong(DrawParams& params) { NOT_IMPLEMENTED_FUNC(); }
		void DrawVertexLitGeneric(DrawParams& params);
		void DrawWeaponSheenPass(DrawParams& params);
		void DrawCloakBlendedPass(DrawParams& params);
		void DrawEmissiveScrollBlendedPass(DrawParams& params);
		void DrawFleshInteriorBlendedPass(DrawParams& params);

		bool CloakBlendedPassIsFullyOpaque(IMaterialVar** params) const;
		bool ShouldDrawMaterialSheen(IMaterialVar** params) const;

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

#if false
	std::vector<const char*> paramNames;
	for (int i = 0; i < GetNumParams(); i++)
	{
		IMaterialVar& p = *params[i];
		paramNames.push_back(p.GetName());
	}
#endif

	InitParamsVertexLitGeneric(params, materialName);

	// Cloak Pass
	if (!params[CLOAKPASSENABLED]->IsDefined())
	{
		params[CLOAKPASSENABLED]->SetIntValue(0);
	}
	else if (params[CLOAKPASSENABLED]->GetIntValue())
	{
		InitParamsCloakBlendedPass(params);
	}

	// Sheen Pass
	if (!params[SHEENPASSENABLED]->IsDefined())
	{
		params[SHEENPASSENABLED]->SetIntValue(0);
	}
	else if (params[SHEENPASSENABLED]->GetIntValue())
	{
		InitParamsWeaponSheenPass(params);
	}

	// Emissive Scroll Pass
	if (!params[EMISSIVEBLENDENABLED]->IsDefined())
	{
		params[EMISSIVEBLENDENABLED]->SetIntValue(0);
	}
	else if (params[EMISSIVEBLENDENABLED]->GetIntValue())
	{
		InitParamsEmissiveScrollBlendedPass(params);
	}

	// Flesh Interior Pass
	if (!params[FLESHINTERIORENABLED]->IsDefined())
	{
		params[FLESHINTERIORENABLED]->SetIntValue(0);
	}
	else if (params[FLESHINTERIORENABLED]->GetIntValue())
	{
		InitParamsFleshInteriorBlendedPass(params);
	}
}

void Shader::OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
	const char* materialName)
{
	InitShaderVertexLitGeneric(params);

	// Cloak Pass
	if (params[CLOAKPASSENABLED]->GetIntValue())
		InitShaderCloakBlendedPass(params);

	// TODO : Only do this if we're in range of the camera
	// Weapon Sheen
	if (params[SHEENPASSENABLED]->GetIntValue())
		InitShaderWeaponSheenPass(params);

	// Emissive Scroll Pass
	if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		InitShaderEmissiveScrollBlendedPass(params);

	// Flesh Interior Pass
	if (params[FLESHINTERIORENABLED]->GetIntValue())
		InitShaderFleshInteriorBlendedPass(params);
}

bool Shader::CloakBlendedPassIsFullyOpaque(IMaterialVar** params) const
{
	// TODO: Figure out if this is more complicated
	return params[CLOAKFACTOR]->GetFloatValue() >= 0.9995f;
}

bool Shader::ShouldDrawMaterialSheen(IMaterialVar** params) const
{
	// TODO: Is this more complicated?
	return !!params[SHEENPASSENABLED]->GetIntValue();
}

bool Shader::WantsPhongShader(IMaterialVar** params) const
{
	if (!mat_phong.GetBool())
		return false;

	if (!params[PHONG]->GetBoolValue())
		return false;

	if (params[LIGHTWARPTEXTURE]->IsTexture())
		return true;

	if (params[BASEMAPALPHAPHONGMASK]->GetIntValue() != -1)
	{
		if (!params[BUMPMAP]->IsDefined())
			return false;
	}

	return true;
}

void Shader::DrawWeaponSheenPass(DrawParams& params)
{
	NOT_IMPLEMENTED_FUNC();
}

void Shader::DrawCloakBlendedPass(DrawParams& params)
{
	const bool bBumpMapping = (!g_pConfig->UseBumpmapping()) || !params[BUMPMAP]->IsTexture() ? 0 : 1;

	params.m_UsingRefraction = true;

	if (const auto shadow = params.shadow)
	{
		// Reset shadow state manually since we're drawing from two materials
		//SetInitialShadowState();

		// Set stream format (note that this shader supports compression)
		params.m_Format.AddFlags(VertexFormatFlags::Position | VertexFormatFlags::Normal);

		if (SupportsCompressedVertices())
			params.m_Format.AddFlags(VertexFormatFlags::Meta_Compressed);

		// Textures

		//pShaderShadow->EnableSRGBWrite(true);

		// Blending
		//EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
		//pShaderShadow->EnableAlphaWrites(false);

		// !!! We need to turn this back on because EnableAlphaBlending() above disables it!
		//pShaderShadow->EnableDepthWrites(true);
	}

	if (const auto dynamic = params.dynamic)
	{
		// Reset render state manually since we're drawing from two materials
		//pShaderAPI->SetDefaultState();

		//pShader->SetHWMorphVertexShaderState(VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, SHADER_VERTEXTEXTURE_SAMPLER0);

		params.m_Uniforms.m_CloakFactor = params[CLOAKFACTOR]->GetFloatValue();
		params.m_Uniforms.m_RefractAmount = params[REFRACTAMOUNT]->GetFloatValue();
		params.m_Uniforms.m_RefractColorTint.SetFrom(params[CLOAKCOLORTINT]->GetVecValue());

		// Set c0 and c1 to contain first two rows of ViewProj matrix
		VMatrix mView, mProj;
		dynamic->GetMatrix(MATERIAL_VIEW, mView);
		dynamic->GetMatrix(MATERIAL_PROJECTION, mProj);
		VMatrix mViewProj = mView * mProj;
		mViewProj = mViewProj.Transpose3x3();
		params.m_Uniforms.m_ViewProjR01 = MatrixFromVMatrix<4, 2>(mViewProj);
	}
}

void Shader::DrawEmissiveScrollBlendedPass(DrawParams& params)
{
	NOT_IMPLEMENTED_FUNC();
}

void Shader::DrawFleshInteriorBlendedPass(DrawParams& params)
{
	NOT_IMPLEMENTED_FUNC();
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
	const bool bHasBaseTexture = drawParams.m_SpecConsts.TEXACTIVE_BASETEXTURE = params[BASETEXTURE]->IsTexture();
	const bool bIsAdditive = CShader_IsFlagSet(params.matvars, MATERIAL_VAR_ADDITIVE);
	const bool bHasFlashlight = false;
	const bool bHasBump = drawParams.m_SpecConsts.TEXACTIVE_BUMPMAP = (/*g_pConfig->UseBumpmapping() && */params[BUMPMAP]->IsTexture());
	drawParams.m_SpecConsts.NORMALMAPPING = bHasBump;

	const bool bHasVertexColor = !bVertexLitGeneric && IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
	const bool bHasVertexAlpha = !bVertexLitGeneric && IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);
	drawParams.m_SpecConsts.VERTEXCOLOR = bHasVertexColor || bHasVertexAlpha;

	const bool bSeamlessBase = params[SEAMLESS_BASE]->GetBoolValue();
	const bool bSeamlessDetail = params[SEAMLESS_DETAIL]->GetBoolValue();
	const bool bHasEnvmap = !bHasFlashlight && params[ENVMAP]->IsTexture();
	const bool bHasNormal = IsPC() || bVertexLitGeneric || bHasEnvmap || bHasFlashlight || bSeamlessBase || bSeamlessDetail;

	const bool bSRGBWrite = !params[LINEARWRITE]->GetBoolValue();
	drawParams.m_SpecConsts.GAMMA_CONVERT_VERTEX_COLOR = !(!bSRGBWrite && bHasVertexColor);

	static constexpr auto SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
	static constexpr auto SAMPLER_BUMPMAP = SHADER_SAMPLER1;
	static constexpr auto SAMPLER_REFRACT = SHADER_SAMPLER2;

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

		if (bHasBaseTexture)
			shadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);

		if (drawParams.m_UsingRefraction)
			shadow->EnableSRGBRead(SAMPLER_REFRACT, true);

		if (bHasBump)
			shadow->EnableSRGBRead(SAMPLER_BUMPMAP, false);

		shadow->EnableSRGBWrite(bSRGBWrite);
	}

	if (const auto dynamic = params.dynamic)
	{
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

		if (bHasBaseTexture)
		{
			BindTexture(SAMPLER_BASETEXTURE, BASETEXTURE, FRAME);
			drawParams.m_Uniforms.m_BaseTextureTransform = TextureTransform(params[BASETEXTURETRANSFORM]->GetMatrixValue());
		}

		if (drawParams.m_UsingRefraction)
			dynamic->BindStandardTexture(SAMPLER_REFRACT, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

		if (bHasBump)
		{
			BindTexture(SAMPLER_BUMPMAP, BUMPMAP, BUMPFRAME);
			drawParams.m_Uniforms.m_BumpTransform = TextureTransform(params[BUMPTRANSFORM]->GetMatrixValue());
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

bool Shader::WantsSkinShader(IMaterialVar** params) const
{
	LOG_FUNC();

	if (!params[PHONG]->GetIntValue())
		return false; // No skin without phong

	if (params[LIGHTWARPTEXTURE]->IsTexture())
		return true; // If phong && diffuse warp, do skin

	if (params[BASEMAPALPHAPHONGMASK]->GetIntValue() != 1)
	{
		if (!params[BUMPMAP]->IsTexture())
			return false; // Don't use if texture isn't specified
	}

	return true;
}

void Shader::InitParamsVertexLitGeneric(IMaterialVar** params, const char* materialName) const
{
	LOG_FUNC();

	InitIntParam(PHONG, params, 0);

	InitFloatParam(ALPHATESTREFERENCE, params, 0.0f);
	InitIntParam(VERTEXALPHATEST, params, 0);

	InitIntParam(FLASHLIGHTNOLAMBERT, params, 0);

	InitVecParam(DETAILTINT, params, 1, 1, 1);

	InitVecParam(ENVMAPTINT, params, 1, 1, 1);

	InitIntParam(ENVMAPFRAME, params, 0);
	InitIntParam(BUMPFRAME, params, 0);
	InitFloatParam(DETAILBLENDFACTOR, params, 1.0);
	InitIntParam(RECEIVEFLASHLIGHT, params, 0);

	InitFloatParam(DETAILSCALE, params, 4.0f);

	InitIntParam(BLENDTINTBYBASEALPHA, params, 0);
	InitFloatParam(BLENDTINTCOLOROVERBASE, params, 0);

	InitVecParam(SELFILLUMTINT, params, 1, 1, 1);

	if (WantsSkinShader(params))
	{
		if (!g_pHardwareConfig->SupportsPixelShaders_2_b() || !g_pConfig->UsePhong())
		{
			params[PHONG]->SetIntValue(0);
		}
		else
		{
			InitParamsSkin(params);
			return;
		}
	}

	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	if (g_pHardwareConfig->SupportsBorderColor())
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
	}

	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if (g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined() && params[ALBEDO]->IsDefined() &&
		params[BASETEXTURE]->IsDefined())
	{
		params[BASETEXTURE]->SetStringValue(params[ALBEDO]->GetStringValue());
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

	InitIntParam(ENVMAPMASKFRAME, params, 0);
	InitFloatParam(ENVMAPCONTRAST, params, 0.0);
	InitFloatParam(ENVMAPSATURATION, params, 1.0f);
	InitFloatParam(SEAMLESS_SCALE, params, 0.0);

	// handle line art parms
	InitFloatParam(EDGESOFTNESSSTART, params, 0.5);
	InitFloatParam(EDGESOFTNESSEND, params, 0.5);
	InitFloatParam(GLOWALPHA, params, 1.0);
	InitFloatParam(OUTLINEALPHA, params, 1.0);

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

	bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
	if (hasNormalMapAlphaEnvmapMask)
	{
		params[ENVMAPMASK]->SetUndefined();
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

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

	InitFloatParam(HDRCOLORSCALE, params, 1.0f);

	InitIntParam(LINEARWRITE, params, 0);
	InitIntParam(GAMMACOLORREAD, params, 0);

	InitIntParam(DEPTHBLEND, params, 0);
	InitFloatParam(DEPTHBLENDSCALE, params, 50.0f);
}

void Shader::InitParamsSkin(IMaterialVar** params) const
{
	LOG_FUNC();

	if (g_pHardwareConfig->SupportsBorderColor())
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
	}

	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if (g_pConfig->UseBumpmapping() &&
		params[BUMPMAP]->IsDefined() &&
		params[ALBEDO]->IsDefined() &&
		params[BASETEXTURE]->IsDefined())
	{
		params[BASETEXTURE]->SetStringValue(params[ALBEDO]->GetStringValue());
	}

	// This shader can be used with hw skinning
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);

	// No texture means no env mask in base alpha
	if (!params[BASETEXTURE]->IsDefined())
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

	// Lots of reasons to want tangent space, since we bind a flat normal map in many cases where we don't have a bump map
	bool bBump = g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined();
	bool bEnvMap = params[ENVMAP]->IsDefined();
	bool bDiffuseWarp = params[LIGHTWARPTEXTURE]->IsDefined();
	bool bPhong = params[PHONG]->IsDefined();
	if (bBump || bEnvMap || bDiffuseWarp || bPhong)
	{
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}
	else
	{
		CLEAR_FLAGS(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
	}

	InitIntParam(SELFILLUMFRESNEL, params, 0);

	InitVecParam(SELFILLUMFRESNELMINMAXEXP, params, 0, 1, 1);

	InitIntParam(BASEMAPALPHAPHONGMASK, params, 0);

	InitFloatParam(ENVMAPFRESNEL, params, 0);
}

void Shader::InitParamsCloakBlendedPass(IMaterialVar** params) const
{
	LOG_FUNC();

	// Set material flags
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	SET_FLAGS(MATERIAL_VAR_MODEL);
	SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// Set material parameter default values
	InitFloatParam(CLOAKFACTOR, params, DEFAULT_CLOAK_FACTOR);
	InitFloatParam(REFRACTAMOUNT, params, DEFAULT_REFRACT_AMOUNT);
	InitVecParam(CLOAKCOLORTINT, params, DEFAULT_CLOAK_COLOR_TINT.x, DEFAULT_CLOAK_COLOR_TINT.y, DEFAULT_CLOAK_COLOR_TINT.z, DEFAULT_CLOAK_COLOR_TINT.w);
	InitIntParam(BUMPFRAME, params, 0);
}

void Shader::InitParamsWeaponSheenPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void Shader::InitParamsEmissiveScrollBlendedPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC();
}

void Shader::InitParamsFleshInteriorBlendedPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC();
}

void Shader::InitShaderSkin(IMaterialVar** params)
{
	LOG_FUNC();

	LoadTexture(FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB);

	bool bIsBaseTextureTranslucent = false;
	if (params[BASETEXTURE]->IsDefined())
	{
		LoadTexture(BASETEXTURE, TEXTUREFLAGS_SRGB);

		if (params[BASETEXTURE]->GetTextureValue()->IsTranslucent())
			bIsBaseTextureTranslucent = true;

		if (params[COMPRESS]->IsDefined() && params[STRETCH]->IsDefined())
		{
			LoadTexture(COMPRESS, TEXTUREFLAGS_SRGB);
			LoadTexture(STRETCH, TEXTUREFLAGS_SRGB);
		}
	}

	bool bHasSelfIllumMask = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && params[SELFILLUMMASK]->IsDefined();

	// No alpha channel in any of the textures? No self illum or envmapmask
	if (!bIsBaseTextureTranslucent)
	{
		bool bHasSelfIllumFresnel = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && (params[SELFILLUMFRESNEL]->GetIntValue() != 0);

		// Can still be self illum with no base alpha if using one of these alternate modes
		if (!bHasSelfIllumFresnel && !bHasSelfIllumMask)
			CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);

		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	if (params[PHONG]->IsDefined())
	{
		LoadTexture(PHONGEXPONENTTEXTURE);
		LoadTexture(LIGHTWARPTEXTURE);
		LoadTexture(PHONGWARPTEXTURE);
	}

	if (params[DETAIL]->IsDefined())
	{
		int nDetailBlendMode = params[DETAILBLENDMODE]->GetIntValue();
		if (nDetailBlendMode == 0) // Mod2X
			LoadTexture(DETAIL);
		else
			LoadTexture(DETAIL, TEXTUREFLAGS_SRGB);
	}

	if (g_pConfig->UseBumpmapping())
	{
		if (params[BUMPMAP]->IsDefined())
		{
			LoadBumpMap(BUMPMAP);
			SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);

			if (params[COMPRESS]->IsDefined() && params[STRETCH]->IsDefined())
			{
				LoadTexture(COMPRESS);
				LoadTexture(STRETCH);
			}
		}
	}

	if (params[ENVMAP]->IsDefined())
		LoadCubeMap(ENVMAP, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0);

	if (bHasSelfIllumMask)
		LoadTexture(SELFILLUMMASK);
}

void Shader::InitShaderVertexLitGeneric(IMaterialVar** params)
{
	LOG_FUNC();

	// both detailed and bumped = needs skin shader (for now)
	bool bNeedsSkinBecauseOfDetail = false;

	//bool bHasBump = ( info.m_nBumpmap != -1 ) && params[info.m_nBumpmap]->IsTexture();
	//if ( bHasBump )
	//{
	//	if (  ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsDefined() )
	//		bNeedsSkinBecauseOfDetail = true;
	//}

	if (bNeedsSkinBecauseOfDetail || (params[PHONG]->GetIntValue() && g_pHardwareConfig->SupportsPixelShaders_2_b()))
		return InitShaderSkin(params);

	LoadTexture(FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB);

	bool bIsBaseTextureTranslucent = false;
	if (params[BASETEXTURE]->IsDefined())
	{
		LoadTexture(BASETEXTURE, (params[GAMMACOLORREAD]->GetIntValue() == 1) ? 0 : TEXTUREFLAGS_SRGB);

		if (params[BASETEXTURE]->GetTextureValue()->IsTranslucent())
			bIsBaseTextureTranslucent = true;
	}

	bool bHasSelfIllumMask = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && params[SELFILLUMMASK]->IsDefined();

	// No alpha channel in any of the textures? No self illum or envmapmask
	if (!bIsBaseTextureTranslucent)
	{
		bool bHasSelfIllumFresnel = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && (params[SELFILLUMFRESNEL]->GetIntValue() != 0);

		// Can still be self illum with no base alpha if using one of these alternate modes
		if (!bHasSelfIllumFresnel && !bHasSelfIllumMask)
			CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);

		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	if (params[DETAIL]->IsDefined())
	{
		int nDetailBlendMode = (DETAILBLENDMODE == -1) ? 0 : params[DETAILBLENDMODE]->GetIntValue();

		if (nDetailBlendMode == 0) //Mod2X
			LoadTexture(DETAIL);
		else
			LoadTexture(DETAIL, TEXTUREFLAGS_SRGB);
	}

	if (g_pConfig->UseBumpmapping())
	{
		if (params[BUMPMAP]->IsDefined())
		{
			LoadBumpMap(BUMPMAP);
			SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
		}
		else if (params[LIGHTWARPTEXTURE]->IsDefined())
		{
			SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
		}
	}

	// Don't alpha test if the alpha channel is used for other purposes
	if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
	{
		CLEAR_FLAGS(MATERIAL_VAR_ALPHATEST);
	}

	if (params[ENVMAP]->IsDefined())
	{
		if (!IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE))
			LoadCubeMap(ENVMAP, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0);
		else
			LoadTexture(ENVMAP, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0);

		if (!g_pHardwareConfig->SupportsCubeMaps())
			SET_FLAGS(MATERIAL_VAR_ENVMAPSPHERE);
	}

	if (params[ENVMAPMASK]->IsDefined())
		LoadTexture(ENVMAPMASK);

	if (params[LIGHTWARPTEXTURE]->IsDefined())
		LoadTexture(LIGHTWARPTEXTURE);

	if (bHasSelfIllumMask)
		LoadTexture(SELFILLUMMASK);
}

void Shader::InitShaderCloakBlendedPass(IMaterialVar** params)
{
	LOG_FUNC();

	if (g_pConfig->UseBumpmapping())
	{
		//LoadBumpMap(BUMPMAP);
		LoadTexture(BUMPMAP); // FIXME: Is this a bug in valve's code? Should this be LoadBumpMap?
	}
}

void Shader::InitShaderWeaponSheenPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void Shader::InitShaderEmissiveScrollBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}

void Shader::InitShaderFleshInteriorBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}
