#include "BaseShaderNext.h"
#include "ParamGroups.h"

#include "UniformBufConstructs/TextureTransform.h"

#include <stdshader_vulkan/ShaderData.h>
#include <TF2Vulkan/AlignedTypes.h>
#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/ISpecConstLayout.h>
#include <TF2Vulkan/IUniformBufferPool.h>
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
	};

	struct SpecConstBuf final : BaseSpecConstBuffer<SpecConstBuf>
	{
		bool1 VERTEXCOLOR;
		bool1 SKINNING;
		bool1 COMPRESSED_VERTS;
		bool32 DIFFUSELIGHTING;
		bool32 DONT_GAMMA_CONVERT_VERTEX_COLOR;

		bool32 TEXACTIVE_BASETEXTURE;
		bool32 TEXACTIVE_BUMPMAP;
	};

	struct SpecConstLayout final : BaseSpecConstLayout<SpecConstLayout, SpecConstBuf>
	{
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, VERTEXCOLOR);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, SKINNING);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, COMPRESSED_VERTS);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, DIFFUSELIGHTING);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, DONT_GAMMA_CONVERT_VERTEX_COLOR);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, TEXACTIVE_BASETEXTURE);
		SPEC_CONST_BUF_ENTRY(SpecConstBuf, TEXACTIVE_BUMPMAP);

	} static constexpr s_SpecConstLayout;

	class Shader : public ShaderNext<Shader, Params>
	{
	public:
		void OnInitShader(IShaderNextFactory& instanceMgr) override;
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
			const char* materialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;

		virtual bool IsVertexLitGeneric() const { return false; }

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
			ShaderDataCommon m_UniformsCommon;
			VSModelMatrices m_ModelMatrices;
			UniformBuf m_Uniforms;

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

		IUniformBufferPool* m_UBufCommon = nullptr;
		UniformBufferIndex m_UBufCommonIndex = UniformBufferIndex::Invalid;

		IUniformBufferPool* m_UBufModelMatrices = nullptr;
		UniformBufferIndex m_UBufModelMatricesIndex = UniformBufferIndex::Invalid;

		IUniformBufferPool* m_UniformBuf = nullptr;
		UniformBufferIndex m_UniformBufferIndex = UniformBufferIndex::Invalid;
	};

	class UnlitGeneric final : public Shader
	{
	public:
		const char* GetName() const override { return "UnlitGeneric"; }
	};

	class VertexLitGeneric final : public Shader
	{
	public:
		const char* GetName() const override { return "VertexLitGeneric"; }

		bool IsVertexLitGeneric() const override { return true; }
	};

	class Wireframe final : public Shader
	{
	public:
		const char* GetName() const override { return "Wireframe"; }
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
	};

	static const DefaultInstanceRegister<UnlitGeneric> s_UnlitGeneric;
	static const DefaultInstanceRegister<VertexLitGeneric> s_VertexLitGeneric;
	static const DefaultInstanceRegister<Wireframe> s_Wireframe;
}

DEFINE_NSHADER_FALLBACK(UnlitGeneric_DX8, UnlitGeneric);
DEFINE_NSHADER_FALLBACK(Wireframe_DX8, Wireframe);
DEFINE_NSHADER_FALLBACK(Wireframe_DX9, Wireframe);
DEFINE_NSHADER_FALLBACK(DebugMorphAccumulator, UnlitGeneric);
//DEFINE_NSHADER_FALLBACK(VertexLitGeneric_DX8, VertexLitGeneric);

static ConVar mat_phong("mat_phong", "1");
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar r_lightwarpidentity("r_lightwarpidentity", "0", FCVAR_CHEAT);
static ConVar mat_luxels("mat_luxels", "0", FCVAR_CHEAT);

void Wireframe::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	SET_FLAGS(MATERIAL_VAR_NOFOG);
	SET_FLAGS(MATERIAL_VAR_WIREFRAME);

	Shader::OnInitShaderParams(params, materialName);
}

void Shader::OnInitShader(IShaderNextFactory& instanceMgr)
{
	LOG_FUNC();

	m_VSShader = &instanceMgr.FindOrCreateShaderGroup(ShaderType::Vertex, "xlitgeneric_vs", s_SpecConstLayout);
	m_PSShader = &instanceMgr.FindOrCreateShaderGroup(ShaderType::Pixel, "xlitgeneric_ps", s_SpecConstLayout);

	m_UBufCommon = &instanceMgr.FindOrCreateUniformBuf(sizeof(ShaderDataCommon));
	m_UBufCommonIndex = m_VSShader->FindUniformBuffer(UniformBufferStandardType::ShaderCommon);

	m_UBufModelMatrices = &instanceMgr.FindOrCreateUniformBuf(sizeof(VSModelMatrices));
	m_UBufModelMatricesIndex = m_VSShader->FindUniformBuffer(UniformBufferStandardType::VSModelMatrices);

	m_UniformBuf = &instanceMgr.FindOrCreateUniformBuf(sizeof(UniformBuf));
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

void Shader::DrawVertexLitGeneric(DrawParams& params)
{
	params.m_Format.AddFlags(VertexFormatFlags::Position);

	if (params.shadow)
	{

	}

	if (params.dynamic)
	{

	}
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
		params.m_Format.AddFlags(VertexFormatFlags::Position | VertexFormatFlags::Normal | VertexFormatFlags::Meta_Compressed);

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

	const bool bVertexLitGeneric = IsVertexLitGeneric();
	const bool hasDiffuseLighting = drawParams.m_SpecConsts.DIFFUSELIGHTING = bVertexLitGeneric;
	const bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
	const bool bHasBaseTexture = drawParams.m_SpecConsts.TEXACTIVE_BASETEXTURE = params[BASETEXTURE]->IsTexture();
	const bool bHasBump = drawParams.m_SpecConsts.TEXACTIVE_BUMPMAP = (g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsTexture());
	const bool bIsAdditive = CShader_IsFlagSet(params.matvars, MATERIAL_VAR_ADDITIVE);
	const bool bHasFlashlight = false;

	const bool bHasVertexColor = bVertexLitGeneric ? false : IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
	const bool bHasVertexAlpha = bVertexLitGeneric ? false : IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);
	drawParams.m_SpecConsts.VERTEXCOLOR = bHasVertexColor || bHasVertexAlpha;

	const bool bSRGBWrite = !params[LINEARWRITE]->GetBoolValue();
	drawParams.m_SpecConsts.DONT_GAMMA_CONVERT_VERTEX_COLOR = !bSRGBWrite && bHasVertexColor;

	static constexpr auto SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
	static constexpr auto SAMPLER_BUMPMAP = SHADER_SAMPLER1;
	static constexpr auto SAMPLER_REFRACT = SHADER_SAMPLER2;

	if (const auto shadow = params.shadow)
	{
		shadow->SetShaders(m_VSShader, m_PSShader);

		// Alpha blending
		SetBlendingShadowState(EvaluateBlendRequirements());

		drawParams.m_Format.AddFlags(VertexFormatFlags::Position);

		if (bHasVertexAlpha || bHasVertexColor)
			drawParams.m_Format.AddFlags(VertexFormatFlags::Color);

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
		[[maybe_unused]] auto& custom = drawParams.m_Uniforms;
		dynamic->GetWorldSpaceCameraPosition(common.m_EyePos);

		// Base Matrices
		{
			VMatrix model, view, proj;
			dynamic->GetMatrix(MATERIAL_MODEL, model);
			dynamic->GetMatrix(MATERIAL_VIEW, view);
			dynamic->GetMatrix(MATERIAL_PROJECTION, proj);

			common.m_ViewProj = proj * view;
			common.m_ModelViewProj = (common.m_ViewProj * model).Transpose();
			common.m_ViewProj = common.m_ViewProj.Transpose();
		}

		// Model matrices
		{
			VMatrix tmp;
			dynamic->GetMatrix(MATERIAL_MODEL, tmp);
			modelMats.m_Model[0] = tmp.Transpose().As3x4();
		}

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
		params.dynamic->BindUniformBuffer(m_UniformBuf->Create().Update(drawParams.m_Uniforms), m_UniformBufferIndex);
		params.dynamic->BindUniformBuffer(m_UBufCommon->Create().Update(drawParams.m_UniformsCommon), m_UBufCommonIndex);
		params.dynamic->BindUniformBuffer(m_UBufModelMatrices->Create().Update(drawParams.m_ModelMatrices), m_UBufModelMatricesIndex);

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

	if (IsVertexLitGeneric())
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
	NOT_IMPLEMENTED_FUNC();
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
	NOT_IMPLEMENTED_FUNC();
}

void Shader::InitShaderEmissiveScrollBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}

void Shader::InitShaderFleshInteriorBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}


#if false
struct DummyContextData : CBasePerMaterialContextData
{

};
void Shader::DrawFleshInteriorBlendedPass(DrawParams& params)
{
	//CVertexLitGeneric_DX9_Context* pContextData = reinterpret_cast<CVertexLitGeneric_DX9_Context*> (*pContextDataPtr);
	DummyContextData* pContextData = static_cast<DummyContextData*>(*params.context);

	const bool bHasFlashlight = false; // TODO

	const bool bHasBump = params[BUMPMAP]->IsTexture();
	const bool bIsDecal = IS_FLAG_SET(MATERIAL_VAR_DECAL);

	const bool bVertexLitGeneric = IsVertexLitGeneric();
	const bool hasDiffuseLighting = bVertexLitGeneric;
	const bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
	const bool bHasDiffuseWarp = hasDiffuseLighting && params[LIGHTWARPTEXTURE]->IsTexture();
	const bool bHasLightmapTexture = params[LIGHTMAP]->IsTexture();
	const bool bHasMatLuxel = bHasLightmapTexture && mat_luxels.GetBool();
	const bool bFlashlightNoLambert = params[FLASHLIGHTNOLAMBERT]->GetBoolValue();
	const bool bAmbientOnly = params[AMBIENTONLY]->GetBoolValue();

	const float fBlendFactor = params[DETAILBLENDFACTOR]->GetFloatValue();
	const bool bHasDetailTexture = params[DETAIL]->IsTexture();
	const int nDetailBlendMode = bHasDetailTexture ? params[DETAILBLENDMODE]->GetIntValue() : 0;
	const int nDetailTranslucencyTexture =
		(bHasDetailTexture && (nDetailBlendMode == 3 || nDetailBlendMode == 8 || nDetailBlendMode == 9)) ? DETAIL : -1;

	const bool bBlendTintByBaseAlpha = params[BLENDTINTBYBASEALPHA]->GetBoolValue();
	const float fTintReplaceFactor = params[BLENDTINTCOLOROVERBASE]->GetFloatValue();

	BlendType_t nBlendType;
	const bool bHasBaseTexture = params[BASETEXTURE]->IsTexture();
	if (bHasBaseTexture)
	{
		// if base alpha is used for tinting, ignore the base texture for computing translucency
		nBlendType = EvaluateBlendRequirements(bBlendTintByBaseAlpha ? -1 : BASETEXTURE, true, nDetailTranslucencyTexture);
	}
	else
	{
		nBlendType = EvaluateBlendRequirements(ENVMAPMASK, false);
	}

	const bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested && !bHasFlashlight; //dest alpha is free for special use

	const bool bHasEnvmap = (!bHasFlashlight) && params[ENVMAP]->IsTexture();

	const bool bHasVertexColor = bVertexLitGeneric ? false : IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
	bool bHasVertexAlpha = bVertexLitGeneric ? false : IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);

	if (IsSnapshotting() || (!pContextData) || (pContextData->m_bMaterialVarsChanged))
	{
		const bool bSeamlessBase = params[SEAMLESS_BASE]->GetBoolValue();
		const bool bSeamlessDetail = params[SEAMLESS_DETAIL]->GetBoolValue();
		const bool bDistanceAlpha = params[DISTANCEALPHA]->GetBoolValue();
		const bool bHasSelfIllum = !bHasFlashlight && IS_FLAG_SET(MATERIAL_VAR_SELFILLUM);
		const bool bHasEnvmapMask = !bHasFlashlight && params[ENVMAPMASK]->IsTexture();
		const bool bHasSelfIllumFresnel = bHasDetailTexture && bHasSelfIllum && params[SELFILLUMFRESNEL]->GetBoolValue();

		const bool bHasSelfIllumMask = bHasSelfIllum && params[SELFILLUMMASK]->IsTexture();
		const bool hasSelfIllumInEnvMapMask = params[SELFILLUM_ENVMAPMASK_ALPHA]->GetFloatValue() != 0.0;

		if (IsSnapshotting())
		{
			/*^*/ // 		printf("\t\t[2] snapshotting...\n");

			bool hasBaseAlphaEnvmapMask = IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);

			if (params[VERTEXALPHATEST]->GetIntValue() > 0)
				bHasVertexAlpha = true;

			// look at color and alphamod stuff.
			// Unlit generic never uses the flashlight
			if (bHasSelfIllumFresnel)
			{
				CLEAR_FLAGS(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
				hasNormalMapAlphaEnvmapMask = false;
			}

			bool bHasEnvmap = (!bHasFlashlight || IsX360()) && (info.m_nEnvmap != -1) && params[info.m_nEnvmap]->IsTexture();
			bool bHasLegacyEnvSphereMap = bHasEnvmap && IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE);
			bool bHasNormal = bVertexLitGeneric || bHasEnvmap || bHasFlashlight || bSeamlessBase || bSeamlessDetail;
			if (IsPC())
			{
				// On PC, LIGHTING_PREVIEW requires normals (they won't use much memory - unlitgeneric isn't used on many models)
				bHasNormal = true;
			}

			bool bHalfLambert = IS_FLAG_SET(MATERIAL_VAR_HALFLAMBERT);
			// Alpha test: FIXME: shouldn't this be handled in CBaseVSShader::SetInitialShadowState
			pShaderShadow->EnableAlphaTest(bIsAlphaTested);

			if (info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f)
			{
				pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue());
			}

			int nShadowFilterMode = 0;
			if (bHasFlashlight)
			{
				if (g_pHardwareConfig->SupportsPixelShaders_2_b())
				{
					nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
				}

				if (!IsX360())
				{
					if (params[info.m_nBaseTexture]->IsTexture())
					{
						SetAdditiveBlendingShadowState(info.m_nBaseTexture, true);
					}
					else
					{
						SetAdditiveBlendingShadowState(info.m_nEnvmapMask, false);
					}

					if (bIsAlphaTested)
					{
						// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to
						// be the same on both the regular pass and the flashlight pass.
						pShaderShadow->EnableAlphaTest(false);
						pShaderShadow->DepthFunc(SHADER_DEPTHFUNC_EQUAL);
					}

					// Be sure not to write to dest alpha
					pShaderShadow->EnableAlphaWrites(false);

					pShaderShadow->EnableBlending(true);
					pShaderShadow->EnableDepthWrites(false);
				}
				else
				{
					SetBlendingShadowState(nBlendType);
				}
			}
			else
			{
				SetBlendingShadowState(nBlendType);
			}

			unsigned int flags = VERTEX_POSITION;
			if (bHasNormal)
			{
				flags |= VERTEX_NORMAL;
			}
			/*^*/ // 	printf("\t\t[%1d] VERTEX_NORMAL\n",(flags&VERTEX_NORMAL)!=0);

			int userDataSize = 0;
			bool bSRGBInputAdapter = false;

			// basetexture
			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
			if (bHasBaseTexture)
			{
				if ((info.m_nGammaColorRead != -1) && (params[info.m_nGammaColorRead]->GetIntValue() == 1))
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, false);
				else
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, true);

				// If we're on OSX GL on a crappy OS which can't do sRGB from render targets, check to see if we're reading from one...
				if (IsOSX() && !g_pHardwareConfig->CanDoSRGBReadFromRTs())
				{
					ITexture* pBaseTexture = params[info.m_nBaseTexture]->GetTextureValue();
					if (pBaseTexture && pBaseTexture->IsRenderTarget())
					{
						bSRGBInputAdapter = true;
					}
				}
			}

			if (bHasEnvmap)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
				if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
				{
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, true);
				}
			}
			if (bHasFlashlight)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER8, true);	// Depth texture
				pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER8);
				pShaderShadow->EnableTexture(SHADER_SAMPLER6, true);	// Noise map
				pShaderShadow->EnableTexture(SHADER_SAMPLER7, true);	// Flashlight cookie
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER7, true);
				userDataSize = 4; // tangent S
			}

			if (bHasDetailTexture)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
				if (nDetailBlendMode != 0) //Not Mod2X
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, true);
			}

			if (bHasBump || bHasDiffuseWarp)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
				userDataSize = 4; // tangent S
				// Normalizing cube map
				pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);
			}
			if (bHasEnvmapMask)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);
			}

			if (bHasVertexColor || bHasVertexAlpha)
			{
				flags |= VERTEX_COLOR;
			}
			/*^*/ // 	printf("\t\t[%1d] VERTEX_COLOR\n",(flags&VERTEX_COLOR)!=0);
			/*^*/ // 	printf("\t\t[%1d] VERTEX_COLOR_STREAM_1\n",(flags&VERTEX_COLOR_STREAM_1)!=0);


			if (bHasDiffuseWarp && (!bHasFlashlight || IsX360()) && !bHasSelfIllumFresnel)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER9, true);	// Diffuse warp texture
			}

			if ((info.m_nDepthBlend != -1) && (params[info.m_nDepthBlend]->GetIntValue()))
			{
				if (bHasBump)
					Warning("DEPTHBLEND not supported by bump mapped variations of vertexlitgeneric to avoid shader bloat. Either remove the bump map or convince a graphics programmer that it's worth it.\n");

				pShaderShadow->EnableTexture(SHADER_SAMPLER10, true);
			}

			if (bHasSelfIllum)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER11, true);	// self illum mask
			}


			// Always enable this sampler, used for lightmaps depending on the dynamic combo.
			// Lightmaps are generated in gamma space, but not sRGB, so leave that disabled. Conversion is done in the shader.
			pShaderShadow->EnableTexture(SHADER_SAMPLER12, true);

			bool bSRGBWrite = true;
			if ((info.m_nLinearWrite != -1) && (params[info.m_nLinearWrite]->GetIntValue() == 1))
			{
				bSRGBWrite = false;
			}

			pShaderShadow->EnableSRGBWrite(bSRGBWrite);

			// texcoord0 : base texcoord
			int pTexCoordDim[3] = { 2, 2, 3 };
			int nTexCoordCount = 1;

			if (IsBoolSet(info.m_nSeparateDetailUVs, params))
			{
				++nTexCoordCount;
			}
			else
			{
				pTexCoordDim[1] = 0;
			}

#ifndef _X360
			// Special morphed decal information
			if (bIsDecal && g_pHardwareConfig->HasFastVertexTextures())
			{
				nTexCoordCount = 3;
			}
#endif

			// This shader supports compressed vertices, so OR in that flag:
			flags |= VERTEX_FORMAT_COMPRESSED;
			/*^*/ // 		printf("\t\t[%1d] VERTEX_FORMAT_COMPRESSED\n",(flags&VERTEX_FORMAT_COMPRESSED)!=0);

			/*^*/ // 		printf("\t\t      -> CShaderShadowDX8::VertexShaderVertexFormat( flags=%08x, texcount=%d )\n",flags,nTexCoordCount);


			pShaderShadow->VertexShaderVertexFormat(flags, nTexCoordCount, pTexCoordDim, userDataSize);

			if (bHasBump || bHasDiffuseWarp)
			{
#ifndef _X360
				if (!g_pHardwareConfig->HasFastVertexTextures())
#endif
				{
					bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

					DECLARE_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs20);
					SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
					SET_STATIC_VERTEX_SHADER_COMBO(USE_WITH_2B, g_pHardwareConfig->SupportsPixelShaders_2_b());
#ifdef _X360
					SET_STATIC_VERTEX_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
#endif
					SET_STATIC_VERTEX_SHADER_COMBO(USE_STATIC_CONTROL_FLOW, bUseStaticControlFlow);
					SET_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs20);

					if (g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders()) // Always send GL this way
					{
						DECLARE_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps20b);
						SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, bHasEnvmap);
						SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
						SET_STATIC_PIXEL_SHADER_COMBO(LIGHTWARPTEXTURE, bHasDiffuseWarp && !bHasSelfIllumFresnel);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, bHasSelfIllum);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMFRESNEL, bHasSelfIllumFresnel);
						SET_STATIC_PIXEL_SHADER_COMBO(NORMALMAPALPHAENVMAPMASK, hasNormalMapAlphaEnvmapMask && bHasEnvmap);
						SET_STATIC_PIXEL_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
						SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAIL_BLEND_MODE, nDetailBlendMode);
						SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
						SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
						SET_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps20b);
					}
					else // ps_2_0
					{
						DECLARE_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps20);
						SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, bHasEnvmap);
						SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
						SET_STATIC_PIXEL_SHADER_COMBO(LIGHTWARPTEXTURE, bHasDiffuseWarp && !bHasSelfIllumFresnel);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, bHasSelfIllum);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMFRESNEL, bHasSelfIllumFresnel);
						SET_STATIC_PIXEL_SHADER_COMBO(NORMALMAPALPHAENVMAPMASK, hasNormalMapAlphaEnvmapMask && bHasEnvmap);
						SET_STATIC_PIXEL_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
						SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAIL_BLEND_MODE, nDetailBlendMode);
						SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
						SET_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps20);
					}
				}
#ifndef _X360
				else
				{
					// The vertex shader uses the vertex id stream
					SET_FLAGS2(MATERIAL_VAR2_USES_VERTEXID);

					DECLARE_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs30);
					SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
					SET_STATIC_VERTEX_SHADER_COMBO(USE_WITH_2B, true);
					SET_STATIC_VERTEX_SHADER_COMBO(DECAL, bIsDecal);
					SET_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs30);

					DECLARE_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps30);
					SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, bHasEnvmap);
					SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
					SET_STATIC_PIXEL_SHADER_COMBO(LIGHTWARPTEXTURE, bHasDiffuseWarp && !bHasSelfIllumFresnel);
					SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, bHasSelfIllum);
					SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUMFRESNEL, bHasSelfIllumFresnel);
					SET_STATIC_PIXEL_SHADER_COMBO(NORMALMAPALPHAENVMAPMASK, hasNormalMapAlphaEnvmapMask && bHasEnvmap);
					SET_STATIC_PIXEL_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
					SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
					SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
					SET_STATIC_PIXEL_SHADER_COMBO(DETAIL_BLEND_MODE, nDetailBlendMode);
					SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
					SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
					SET_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps30);
				}
#endif
			}
			else // !(bHasBump || bHasDiffuseWarp)
			{
				bool bDistanceAlphaFromDetail = false;
				bool bSoftMask = false;
				bool bGlow = false;
				bool bOutline = false;

				static ConVarRef mat_reduceparticles("mat_reduceparticles");
				bool bDoDepthBlend = IsBoolSet(info.m_nDepthBlend, params) && !mat_reduceparticles.GetBool();

				if (bDistanceAlpha)
				{
					bDistanceAlphaFromDetail = IsBoolSet(info.m_nDistanceAlphaFromDetail, params);
					bSoftMask = IsBoolSet(info.m_nSoftEdges, params);
					bGlow = IsBoolSet(info.m_nGlow, params);
					bOutline = IsBoolSet(info.m_nOutline, params);
				}

#ifndef _X360
				if (!g_pHardwareConfig->HasFastVertexTextures())
#endif
				{
					bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

					DECLARE_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_vs20);
					SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLOR, bHasVertexColor || bHasVertexAlpha);
					SET_STATIC_VERTEX_SHADER_COMBO(CUBEMAP, bHasEnvmap);
					SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
					SET_STATIC_VERTEX_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
					SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_BASE, bSeamlessBase);
					SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_DETAIL, bSeamlessDetail);
					SET_STATIC_VERTEX_SHADER_COMBO(SEPARATE_DETAIL_UVS, IsBoolSet(info.m_nSeparateDetailUVs, params));
					SET_STATIC_VERTEX_SHADER_COMBO(USE_STATIC_CONTROL_FLOW, bUseStaticControlFlow);
					SET_STATIC_VERTEX_SHADER_COMBO(DONT_GAMMA_CONVERT_VERTEX_COLOR, (!bSRGBWrite) && bHasVertexColor);
					SET_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_vs20);

					if (g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders()) // Always send Gl this way
					{
						DECLARE_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps20b);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM_ENVMAPMASK_ALPHA, (hasSelfIllumInEnvMapMask && (bHasEnvmapMask)));
						SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, bHasEnvmap);
						SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP_SPHERE_LEGACY, bHasLegacyEnvSphereMap);
						SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
						SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMASK, bHasEnvmapMask);
						SET_STATIC_PIXEL_SHADER_COMBO(BASEALPHAENVMAPMASK, hasBaseAlphaEnvmapMask);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, bHasSelfIllum);
						SET_STATIC_PIXEL_SHADER_COMBO(VERTEXCOLOR, bHasVertexColor);
						SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAIL_BLEND_MODE, nDetailBlendMode);
						SET_STATIC_PIXEL_SHADER_COMBO(SEAMLESS_BASE, bSeamlessBase);
						SET_STATIC_PIXEL_SHADER_COMBO(SEAMLESS_DETAIL, bSeamlessDetail);
						SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHA, bDistanceAlpha);
						SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHAFROMDETAIL, bDistanceAlphaFromDetail);
						SET_STATIC_PIXEL_SHADER_COMBO(SOFT_MASK, bSoftMask);
						SET_STATIC_PIXEL_SHADER_COMBO(OUTLINE, bOutline);
						SET_STATIC_PIXEL_SHADER_COMBO(OUTER_GLOW, bGlow);
						SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
						SET_STATIC_PIXEL_SHADER_COMBO(DEPTHBLEND, bDoDepthBlend);
						SET_STATIC_PIXEL_SHADER_COMBO(SRGB_INPUT_ADAPTER, bSRGBInputAdapter ? 1 : 0);
						SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
						SET_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps20b);
					}
					else // ps_2_0
					{
						DECLARE_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps20);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM_ENVMAPMASK_ALPHA, (hasSelfIllumInEnvMapMask && (bHasEnvmapMask)));
						SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, bHasEnvmap);
						SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP_SPHERE_LEGACY, bHasLegacyEnvSphereMap);
						SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
						SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMASK, bHasEnvmapMask);
						SET_STATIC_PIXEL_SHADER_COMBO(BASEALPHAENVMAPMASK, hasBaseAlphaEnvmapMask);
						SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, bHasSelfIllum);
						SET_STATIC_PIXEL_SHADER_COMBO(VERTEXCOLOR, bHasVertexColor);
						SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
						SET_STATIC_PIXEL_SHADER_COMBO(DETAIL_BLEND_MODE, nDetailBlendMode);
						SET_STATIC_PIXEL_SHADER_COMBO(SEAMLESS_BASE, bSeamlessBase);
						SET_STATIC_PIXEL_SHADER_COMBO(SEAMLESS_DETAIL, bSeamlessDetail);
						SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHA, bDistanceAlpha);
						SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHAFROMDETAIL, bDistanceAlphaFromDetail);
						SET_STATIC_PIXEL_SHADER_COMBO(SOFT_MASK, bSoftMask);
						SET_STATIC_PIXEL_SHADER_COMBO(OUTLINE, bOutline);
						SET_STATIC_PIXEL_SHADER_COMBO(OUTER_GLOW, bGlow);
						SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
						SET_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps20);
					}
				}
#ifndef _X360
				else
				{
					// The vertex shader uses the vertex id stream
					SET_FLAGS2(MATERIAL_VAR2_USES_VERTEXID);

					DECLARE_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_vs30);
					SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLOR, bHasVertexColor || bHasVertexAlpha);
					SET_STATIC_VERTEX_SHADER_COMBO(CUBEMAP, bHasEnvmap);
					SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
					SET_STATIC_VERTEX_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
					SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_BASE, bSeamlessBase);
					SET_STATIC_VERTEX_SHADER_COMBO(SEAMLESS_DETAIL, bSeamlessDetail);
					SET_STATIC_VERTEX_SHADER_COMBO(SEPARATE_DETAIL_UVS, IsBoolSet(info.m_nSeparateDetailUVs, params));
					SET_STATIC_VERTEX_SHADER_COMBO(DECAL, bIsDecal);
					SET_STATIC_VERTEX_SHADER_COMBO(DONT_GAMMA_CONVERT_VERTEX_COLOR, bSRGBWrite ? 0 : 1);
					SET_STATIC_VERTEX_SHADER(vertexlit_and_unlit_generic_vs30);

					DECLARE_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps30);
					SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM_ENVMAPMASK_ALPHA, (hasSelfIllumInEnvMapMask && (bHasEnvmapMask)));
					SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, bHasEnvmap);
					SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP_SPHERE_LEGACY, bHasLegacyEnvSphereMap);
					SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
					SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMASK, bHasEnvmapMask);
					SET_STATIC_PIXEL_SHADER_COMBO(BASEALPHAENVMAPMASK, hasBaseAlphaEnvmapMask);
					SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, bHasSelfIllum);
					SET_STATIC_PIXEL_SHADER_COMBO(VERTEXCOLOR, bHasVertexColor);
					SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
					SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bHasDetailTexture);
					SET_STATIC_PIXEL_SHADER_COMBO(DETAIL_BLEND_MODE, nDetailBlendMode);
					SET_STATIC_PIXEL_SHADER_COMBO(SEAMLESS_BASE, bSeamlessBase);
					SET_STATIC_PIXEL_SHADER_COMBO(SEAMLESS_DETAIL, bSeamlessDetail);
					SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHA, bDistanceAlpha);
					SET_STATIC_PIXEL_SHADER_COMBO(DISTANCEALPHAFROMDETAIL, bDistanceAlphaFromDetail);
					SET_STATIC_PIXEL_SHADER_COMBO(SOFT_MASK, bSoftMask);
					SET_STATIC_PIXEL_SHADER_COMBO(OUTLINE, bOutline);
					SET_STATIC_PIXEL_SHADER_COMBO(OUTER_GLOW, bGlow);
					SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
					SET_STATIC_PIXEL_SHADER_COMBO(DEPTHBLEND, bDoDepthBlend);
					SET_STATIC_PIXEL_SHADER_COMBO(BLENDTINTBYBASEALPHA, bBlendTintByBaseAlpha);
					SET_STATIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps30);
				}
#endif
			}

			if (bHasFlashlight && !IsX360())
			{
				pShader->FogToBlack();
			}
			else
			{
				pShader->DefaultFog();
			}

			// HACK HACK HACK - enable alpha writes all the time so that we have them for
			// underwater stuff and the loadout and character select screens.
			pShaderShadow->EnableAlphaWrites(bFullyOpaque);
		}

		if (pShaderAPI && ((!pContextData) || (pContextData->m_bMaterialVarsChanged)))
		{
			/*^*/ // 		printf("\t\t[3] pShaderAPI && ( (! pContextData ) || ( pContextData->m_bMaterialVarsChanged ) )  TRUE \n");
			if (!pContextData)								// make sure allocated
			{
				++g_nSnapShots;
				pContextData = new CVertexLitGeneric_DX9_Context;
				*pContextDataPtr = pContextData;
			}
			pContextData->m_SemiStaticCmdsOut.Reset();
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderFogParams(21);
			if (bHasBaseTexture)
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame);
			}
			else
			{
				if (bHasEnvmap)
				{
					// if we only have an envmap (no basetexture), then we want the albedo to be black.
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER0, TEXTURE_BLACK);
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER0, TEXTURE_WHITE);
				}
			}
			if (bHasDetailTexture)
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER2, info.m_nDetail, info.m_nDetailFrame);
			}
			if (bHasSelfIllum)
			{
				if (bHasSelfIllumMask)												// Separate texture for self illum?
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER11, info.m_nSelfIllumMask, -1);	// Bind it
				}
				else																	// else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER11, TEXTURE_BLACK);	// Bind dummy
				}
			}

			if ((info.m_nDepthBlend != -1) && (params[info.m_nDepthBlend]->GetIntValue()))
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER10, TEXTURE_FRAME_BUFFER_FULL_DEPTH);
			}
			if (bSeamlessDetail || bSeamlessBase)
			{
				float flSeamlessData[4] = { params[info.m_nSeamlessScale]->GetFloatValue(),
					0, 0, 0 };
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, flSeamlessData);
			}

			if (info.m_nBaseTextureTransform != -1)
			{
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform);
			}


			if (bHasDetailTexture)
			{
				if (IS_PARAM_DEFINED(info.m_nDetailTextureTransform))
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, info.m_nDetailTextureTransform, info.m_nDetailScale);
				else
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, info.m_nBaseTextureTransform, info.m_nDetailScale);
				//Assert( !bHasBump );
				if (info.m_nDetailTint != -1)
					pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstantGammaToLinear(10, info.m_nDetailTint);
				else
				{
					pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant4(10, 1, 1, 1, 1);
				}
			}
			if (bDistanceAlpha)
			{
				float flSoftStart = GetFloatParam(info.m_nEdgeSoftnessStart, params);
				float flSoftEnd = GetFloatParam(info.m_nEdgeSoftnessEnd, params);
				// set all line art shader parms
				bool bScaleEdges = IsBoolSet(info.m_nScaleEdgeSoftnessBasedOnScreenRes, params);
				bool bScaleOutline = IsBoolSet(info.m_nScaleOutlineSoftnessBasedOnScreenRes, params);

				float flResScale = 1.0;

				float flOutlineStart0 = GetFloatParam(info.m_nOutlineStart0, params);
				float flOutlineStart1 = GetFloatParam(info.m_nOutlineStart1, params);
				float flOutlineEnd0 = GetFloatParam(info.m_nOutlineEnd0, params);
				float flOutlineEnd1 = GetFloatParam(info.m_nOutlineEnd1, params);

				if (bScaleEdges || bScaleOutline)
				{
					int nWidth, nHeight;
					pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
					flResScale = max(0.5, max(1024.0 / nWidth, 768 / nHeight));

					if (bScaleEdges)
					{
						float flMid = 0.5 * (flSoftStart + flSoftEnd);
						flSoftStart = clamp(flMid + flResScale * (flSoftStart - flMid), 0.05, 0.99);
						flSoftEnd = clamp(flMid + flResScale * (flSoftEnd - flMid), 0.05, 0.99);
					}


					if (bScaleOutline)
					{
						// shrink the soft part of the outline, enlarging hard part
						float flMidS = 0.5 * (flOutlineStart1 + flOutlineStart0);
						flOutlineStart1 = clamp(flMidS + flResScale * (flOutlineStart1 - flMidS), 0.05, 0.99);
						float flMidE = 0.5 * (flOutlineEnd1 + flOutlineEnd0);
						flOutlineEnd1 = clamp(flMidE + flResScale * (flOutlineEnd1 - flMidE), 0.05, 0.99);
					}

				}

				float flConsts[] = {
					// c5 - glow values
					GetFloatParam(info.m_nGlowX, params),
					GetFloatParam(info.m_nGlowY, params),
					GetFloatParam(info.m_nGlowStart, params),
					GetFloatParam(info.m_nGlowEnd, params),
					// c6 - glow color
					0, 0, 0,										// will be filled in
					GetFloatParam(info.m_nGlowAlpha, params),
					// c7 - mask range parms
					flSoftStart,
					flSoftEnd,
					0, 0,
					// c8 - outline color
					0, 0, 0,
					GetFloatParam(info.m_nOutlineAlpha, params),
					// c9 - outline parms. ordered for optimal ps20 .wzyx swizzling
					flOutlineStart0,
					flOutlineEnd1,
					flOutlineEnd0,
					flOutlineStart1,
				};

				if (info.m_nGlowColor != -1)
				{
					params[info.m_nGlowColor]->GetVecValue(flConsts + 4, 3);
				}
				if (info.m_nOutlineColor != -1)
				{
					params[info.m_nOutlineColor]->GetVecValue(flConsts + 12, 3);
				}
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant(5, flConsts, 5);

			}
			if (!g_pConfig->m_bFastNoBump)
			{
				if (bHasBump)
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER3, info.m_nBumpmap, info.m_nBumpFrame);
				}
				else if (bHasDiffuseWarp)
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT);
				}
			}
			else
			{
				if (bHasBump)
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT);
				}
			}
			// Setting w to 1 means use separate selfillummask
			float vEnvMapSaturation_SelfIllumMask[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
			if (info.m_nEnvmapSaturation != -1)
				params[info.m_nEnvmapSaturation]->GetVecValue(vEnvMapSaturation_SelfIllumMask, 3);

			vEnvMapSaturation_SelfIllumMask[3] = bHasSelfIllumMask ? 1.0f : 0.0f;
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant(3, vEnvMapSaturation_SelfIllumMask, 1);
			if (bHasEnvmap)
			{
				pContextData->m_SemiStaticCmdsOut.SetEnvMapTintPixelShaderDynamicStateGammaToLinear(0, info.m_nEnvmapTint, fTintReplaceFactor);
			}
			else
			{
				pContextData->m_SemiStaticCmdsOut.SetEnvMapTintPixelShaderDynamicStateGammaToLinear(0, -1, fTintReplaceFactor);
			}

			if (bHasEnvmapMask)
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER4, info.m_nEnvmapMask, info.m_nEnvmapMaskFrame);
			}

			if (bHasSelfIllumFresnel && (!bHasFlashlight || IsX360()))
			{
				float vConstScaleBiasExp[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
				float flMin = IS_PARAM_DEFINED(info.m_nSelfIllumFresnelMinMaxExp) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[0] : 0.0f;
				float flMax = IS_PARAM_DEFINED(info.m_nSelfIllumFresnelMinMaxExp) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[1] : 1.0f;
				float flExp = IS_PARAM_DEFINED(info.m_nSelfIllumFresnelMinMaxExp) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[2] : 1.0f;

				vConstScaleBiasExp[1] = (flMax != 0.0f) ? (flMin / flMax) : 0.0f; // Bias
				vConstScaleBiasExp[0] = 1.0f - vConstScaleBiasExp[1]; // Scale
				vConstScaleBiasExp[2] = flExp; // Exp
				vConstScaleBiasExp[3] = flMax; // Brightness

				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant(11, vConstScaleBiasExp);
			}

			if (bHasDiffuseWarp && (!bHasFlashlight || IsX360()) && !bHasSelfIllumFresnel)
			{
				if (r_lightwarpidentity.GetBool())
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER9, TEXTURE_IDENTITY_LIGHTWARP);
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER9, info.m_nDiffuseWarpTexture, -1);
				}
			}

			if (bHasFlashlight)
			{
				// Tweaks associated with a given flashlight
				VMatrix worldToTexture;
				const FlashlightState_t& flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
				float tweaks[4];
				tweaks[0] = flashlightState.m_flShadowFilterSize / flashlightState.m_flShadowMapResolution;
				tweaks[1] = ShadowAttenFromState(flashlightState);
				pShader->HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
				pShaderAPI->SetPixelShaderConstant(2, tweaks, 1);

				// Dimensions of screen, used for screen-space noise map sampling
				float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
				int nWidth, nHeight;
				pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
				vScreenScale[0] = (float)nWidth / 32.0f;
				vScreenScale[1] = (float)nHeight / 32.0f;
				pShaderAPI->SetPixelShaderConstant(31, vScreenScale, 1);
			}

			if ((!bHasFlashlight || IsX360()) && (info.m_nEnvmapContrast != -1))
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant(2, info.m_nEnvmapContrast);

			// mat_fullbright 2 handling
			bool bLightingOnly = bVertexLitGeneric && mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
			if (bLightingOnly)
			{
				if (bHasBaseTexture)
				{
					if ((bHasSelfIllum && !hasSelfIllumInEnvMapMask))
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER0, TEXTURE_GREY_ALPHA_ZERO);
					}
					else
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER0, TEXTURE_GREY);
					}
				}
				if (bHasDetailTexture)
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER2, TEXTURE_GREY);
				}
			}

			if (bHasBump || bHasDiffuseWarp)
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture(SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED);
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderStateAmbientLightCube(5);
				pContextData->m_SemiStaticCmdsOut.CommitPixelShaderLighting(13);
			}
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant_W(4, info.m_nSelfIllumTint, fBlendFactor);
			pContextData->m_SemiStaticCmdsOut.SetAmbientCubeDynamicStateVertexShader();
			pContextData->m_SemiStaticCmdsOut.End();
		}
	}
	if (pShaderAPI)
	{
		CCommandBufferBuilder< CFixedCommandStorageBuffer< 1000 > > DynamicCmdsOut;
		DynamicCmdsOut.Call(pContextData->m_SemiStaticCmdsOut.Base());

		if (bHasEnvmap)
		{
			DynamicCmdsOut.BindTexture(pShader, SHADER_SAMPLER1, info.m_nEnvmap, info.m_nEnvmapFrame);
		}

		bool bFlashlightShadows = false;
		if (bHasFlashlight)
		{
			VMatrix worldToTexture;
			ITexture* pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
			bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

			if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
			{
				pShader->BindTexture(SHADER_SAMPLER8, pFlashlightDepthTexture, 0);
				DynamicCmdsOut.BindStandardTexture(SHADER_SAMPLER6, TEXTURE_SHADOW_NOISE_2D);
			}

			SetFlashLightColorFromState(state, pShaderAPI, 28, bFlashlightNoLambert);

			Assert(info.m_nFlashlightTexture >= 0 && info.m_nFlashlightTextureFrame >= 0);
			pShader->BindTexture(SHADER_SAMPLER7, state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame);
		}


		// Set up light combo state
		LightState_t lightState = { 0, false, false, false };
		if (bVertexLitGeneric && (!bHasFlashlight || IsX360()))
		{
			pShaderAPI->GetDX9LightState(&lightState);
		}

		// Override the lighting desired if we have a lightmap set!
		if (bHasLightmapTexture)
		{
			lightState.m_bStaticLightVertex = false;
			lightState.m_bStaticLightTexel = true;

			// Usual case, not debugging.
			if (!bHasMatLuxel)
			{
				pShader->BindTexture(SHADER_SAMPLER12, info.m_nLightmap);
			}
			else
			{
				float dimensions[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				DynamicCmdsOut.BindStandardTexture(SHADER_SAMPLER12, TEXTURE_DEBUG_LUXELS);
				pShader->GetTextureDimensions(&dimensions[0], &dimensions[1], info.m_nLightmap);
				DynamicCmdsOut.SetPixelShaderConstant(11, dimensions, 1);
			}
		}

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;
		int numBones = pShaderAPI->GetCurrentNumBones();

		bool bWriteDepthToAlpha;
		bool bWriteWaterFogToAlpha;
		if (bFullyOpaque)
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time.");
		}
		else
		{
			//can't write a special value to dest alpha if we're actually using as-intended alpha
			bWriteDepthToAlpha = false;
			bWriteWaterFogToAlpha = false;
		}

		if (bHasBump || bHasDiffuseWarp)
		{
#ifndef _X360
			if (!g_pHardwareConfig->HasFastVertexTextures())
#endif
			{
				bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

				DECLARE_DYNAMIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs20);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, bUseStaticControlFlow ? 0 : lightState.m_nNumLights);
				SET_DYNAMIC_VERTEX_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_bump_vs20);

				// Bind ps_2_b shader so we can get shadow mapping...
				if (g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders()) // Always send GL this way
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps20b);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(AMBIENT_LIGHT, lightState.m_bAmbientLight ? 1 : 0);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
					//					SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
					SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_bump_ps20b);
				}
				else
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps20);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(AMBIENT_LIGHT, lightState.m_bAmbientLight ? 1 : 0);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
					SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_bump_ps20);
				}
			}
#ifndef _X360
			else
			{
				pShader->SetHWMorphVertexShaderState(VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, VERTEX_SHADER_SHADER_SPECIFIC_CONST_11, SHADER_VERTEXTEXTURE_SAMPLER0);

				DECLARE_DYNAMIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs30);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, pShaderAPI->IsHWMorphingEnabled());
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER(vertexlit_and_unlit_generic_bump_vs30);

				DECLARE_DYNAMIC_PIXEL_SHADER(vertexlit_and_unlit_generic_bump_ps30);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(AMBIENT_LIGHT, lightState.m_bAmbientLight ? 1 : 0);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
				//				SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
				SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_bump_ps30);

				bool bUnusedTexCoords[3] = { false, false, !pShaderAPI->IsHWMorphingEnabled() || !bIsDecal };
				pShaderAPI->MarkUnusedVertexFields(0, 3, bUnusedTexCoords);
			}
#endif
		}
		else // !( bHasBump || bHasDiffuseWarp )
		{
			if (bAmbientOnly)	// Override selected light combo to be ambient only
			{
				lightState.m_bAmbientLight = true;
				lightState.m_bStaticLightVertex = false;
				lightState.m_nNumLights = 0;
			}

#ifndef _X360
			if (!g_pHardwareConfig->HasFastVertexTextures())
#endif
			{
				bool bUseStaticControlFlow = g_pHardwareConfig->SupportsStaticControlFlow();

				DECLARE_DYNAMIC_VERTEX_SHADER(vertexlit_and_unlit_generic_vs20);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMIC_LIGHT, lightState.HasDynamicLight());
				SET_DYNAMIC_VERTEX_SHADER_COMBO(STATIC_LIGHT_VERTEX, lightState.m_bStaticLightVertex ? 1 : 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(STATIC_LIGHT_LIGHTMAP, lightState.m_bStaticLightTexel ? 1 : 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(
					LIGHTING_PREVIEW,
					pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, bUseStaticControlFlow ? 0 : lightState.m_nNumLights);
				SET_DYNAMIC_VERTEX_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_vs20);

				// Bind ps_2_b shader so we can get shadow mapping
				if (g_pHardwareConfig->SupportsPixelShaders_2_b() || g_pHardwareConfig->ShouldAlwaysUseShaderModel2bShaders()) // Always send GL this way
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps20b);

					//					SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
					SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(STATIC_LIGHT_LIGHTMAP, lightState.m_bStaticLightTexel ? 1 : 0);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(DEBUG_LUXELS, bHasMatLuxel ? 1 : 0);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(
						LIGHTING_PREVIEW,
						pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING));
					SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_ps20b);
				}
				else
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps20);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
					SET_DYNAMIC_PIXEL_SHADER_COMBO(STATIC_LIGHT_LIGHTMAP, lightState.m_bStaticLightTexel ? 1 : 0);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(
						LIGHTING_PREVIEW,
						pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING));
					SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_ps20);
				}
			}
#ifndef _X360
			else
			{
				pShader->SetHWMorphVertexShaderState(VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, VERTEX_SHADER_SHADER_SPECIFIC_CONST_11, SHADER_VERTEXTEXTURE_SAMPLER0);

				DECLARE_DYNAMIC_VERTEX_SHADER(vertexlit_and_unlit_generic_vs30);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMIC_LIGHT, lightState.HasDynamicLight());
				SET_DYNAMIC_VERTEX_SHADER_COMBO(STATIC_LIGHT_VERTEX, lightState.m_bStaticLightVertex ? 1 : 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(STATIC_LIGHT_LIGHTMAP, lightState.m_bStaticLightTexel ? 1 : 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW,
					pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(MORPHING, pShaderAPI->IsHWMorphingEnabled());
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_vs30);

				DECLARE_DYNAMIC_PIXEL_SHADER(vertexlit_and_unlit_generic_ps30);
				//				SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(STATIC_LIGHT_LIGHTMAP, lightState.m_bStaticLightTexel ? 1 : 0);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(DEBUG_LUXELS, bHasMatLuxel ? 1 : 0);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(LIGHTING_PREVIEW,
					pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING));
				SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, vertexlit_and_unlit_generic_ps30);

				bool bUnusedTexCoords[3] = { false, false, !pShaderAPI->IsHWMorphingEnabled() || !bIsDecal };
				pShaderAPI->MarkUnusedVertexFields(0, 3, bUnusedTexCoords);
			}
#endif
		}

		if ((info.m_nHDRColorScale != -1) && pShader->IsHDREnabled())
		{
			pShader->SetModulationPixelShaderDynamicState_LinearColorSpace_LinearScale(1, params[info.m_nHDRColorScale]->GetFloatValue());
		}
		else
		{
			pShader->SetModulationPixelShaderDynamicState_LinearColorSpace(1);
		}

		float eyePos[4];
		pShaderAPI->GetWorldSpaceCameraPosition(eyePos);
		DynamicCmdsOut.SetPixelShaderConstant(20, eyePos);

		// Non-bump case does its own depth feathering work
		if (!bHasBump && !bHasDiffuseWarp)
		{
			DynamicCmdsOut.SetDepthFeatheringPixelShaderConstant(13, GetFloatParam(info.m_nDepthBlendScale, params, 50.0f));
		}

		float fPixelFogType = pShaderAPI->GetPixelFogCombo() == 1 ? 1 : 0;
		float fWriteDepthToAlpha = bWriteDepthToAlpha && IsPC() ? 1 : 0;
		float fWriteWaterFogToDestAlpha = (pShaderAPI->GetPixelFogCombo() == 1 && bWriteWaterFogToAlpha) ? 1 : 0;
		float fVertexAlpha = bHasVertexAlpha ? 1 : 0;

		// Controls for lerp-style paths through shader code (bump and non-bump have use different register)
		float vShaderControls[4] = { fPixelFogType, fWriteDepthToAlpha, fWriteWaterFogToDestAlpha, fVertexAlpha };
		DynamicCmdsOut.SetPixelShaderConstant(12, vShaderControls, 1);

		// flashlightfixme: put this in common code.
		if (bHasFlashlight)
		{
			VMatrix worldToTexture;
			const FlashlightState_t& flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
			SetFlashLightColorFromState(flashlightState, pShaderAPI, 28, bFlashlightNoLambert);

			pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, worldToTexture.Base(), 4);

			pShader->BindTexture(SHADER_SAMPLER7, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);

			float atten_pos[8];
			atten_pos[0] = flashlightState.m_fConstantAtten;			// Set the flashlight attenuation factors
			atten_pos[1] = flashlightState.m_fLinearAtten;
			atten_pos[2] = flashlightState.m_fQuadraticAtten;
			atten_pos[3] = flashlightState.m_FarZ;
			atten_pos[4] = flashlightState.m_vecLightOrigin[0];			// Set the flashlight origin
			atten_pos[5] = flashlightState.m_vecLightOrigin[1];
			atten_pos[6] = flashlightState.m_vecLightOrigin[2];
			atten_pos[7] = 1.0f;
			DynamicCmdsOut.SetPixelShaderConstant(22, atten_pos, 2);

			DynamicCmdsOut.SetPixelShaderConstant(24, worldToTexture.Base(), 4);
		}
		DynamicCmdsOut.End();
		pShaderAPI->ExecuteCommandBuffer(DynamicCmdsOut.Base());
	}
	pShader->Draw();

	/*^*/ // 	printf("\t\t<DrawVertexLitGeneric_DX9_Internal\n");
}
#endif
