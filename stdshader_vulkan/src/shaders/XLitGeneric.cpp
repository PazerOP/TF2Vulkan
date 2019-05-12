#include "IVulkanShader.h"

#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <shaderlib/BaseShader.h>
#include <shaderlib/cshader.h>
#include <shaderlib/ShaderDLL.h>

#include <vector>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

namespace
{
	static int s_ParamCount = 0;
	struct XLitGenericParams
	{
		const VulkanShaderParam* GetParams() const { return reinterpret_cast<const VulkanShaderParam*>(this); }

		VSHADER_PARAM(ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)")
		VSHADER_PARAM(COMPRESS, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "compression wrinklemap")
		VSHADER_PARAM(STRETCH, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "expansion wrinklemap")
		VSHADER_PARAM(SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")
		VSHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture")
		VSHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail")
		VSHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture")
		VSHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap")
		VSHADER_PARAM(ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number")
		VSHADER_PARAM(ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask")
		VSHADER_PARAM(ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "")
		VSHADER_PARAM(ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform")
		VSHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint")
		VSHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map")
		VSHADER_PARAM(BUMPCOMPRESS, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "compression bump map")
		VSHADER_PARAM(BUMPSTRETCH, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "expansion bump map")
		VSHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap")
		VSHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform")
		VSHADER_PARAM(ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color")
		VSHADER_PARAM(ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal")
		VSHADER_PARAM(SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "defines that self illum value comes from env map mask alpha")
		VSHADER_PARAM(SELFILLUMFRESNEL, SHADER_PARAM_TYPE_BOOL, "0", "Self illum fresnel")
		VSHADER_PARAM(SELFILLUMFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC4, "0", "Self illum fresnel min, max, exp")
		VSHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
		VSHADER_PARAM(FLASHLIGHTNOLAMBERT, SHADER_PARAM_TYPE_BOOL, "0", "Flashlight pass sets N.L=1.0")
		VSHADER_PARAM(LIGHTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "lightmap texture--will be bound by the engine")

		// Debugging term for visualizing ambient data on its own
		VSHADER_PARAM(AMBIENTONLY, SHADER_PARAM_TYPE_INTEGER, "0", "Control drawing of non-ambient light ()")

		VSHADER_PARAM(PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights")
		VSHADER_PARAM(PHONGTINT, SHADER_PARAM_TYPE_VEC3, "5.0", "Phong tint for local specular lights")
		VSHADER_PARAM(PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1.0", "Apply tint by albedo (controlled by spec exponent texture")
		VSHADER_PARAM(LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term")
		VSHADER_PARAM(PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term")
		VSHADER_PARAM(PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0  0.5  1]", "Parameters for remapping fresnel output")
		VSHADER_PARAM(PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)")
		VSHADER_PARAM(PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map")
		VSHADER_PARAM(PHONGEXPONENTFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "When using a phong exponent texture, this will be multiplied by the 0..1 that comes out of the texture.")
		VSHADER_PARAM(PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting")
		VSHADER_PARAM(BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha")
		VSHADER_PARAM(INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)")
		VSHADER_PARAM(ENVMAPFRESNEL, SHADER_PARAM_TYPE_FLOAT, "0", "Degree to which Fresnel should be applied to env map")
		VSHADER_PARAM(SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum")

		// detail (multi-) texturing
		VSHADER_PARAM(DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade")
		VSHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.")
		VSHADER_PARAM(DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint")
		VSHADER_PARAM(DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform")

		// Rim lighting terms
		VSHADER_PARAM(RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting")
		VSHADER_PARAM(RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights")
		VSHADER_PARAM(RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights")
		VSHADER_PARAM(RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term")

		// Seamless mapping scale
		VSHADER_PARAM(SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model.")
		VSHADER_PARAM(SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture.")
		VSHADER_PARAM(SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch.")

		// Emissive Scroll Pass
		VSHADER_PARAM(EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass")
		VSHADER_PARAM(EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
		VSHADER_PARAM(EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec")
		VSHADER_PARAM(EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength")
		VSHADER_PARAM(EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
		VSHADER_PARAM(EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")
		VSHADER_PARAM(EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map")
		VSHADER_PARAM(TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy")

		// Cloak Pass
		VSHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass")
		VSHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
		VSHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint")
		VSHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "")

		// Weapon Sheen Pass
		VSHADER_PARAM(SHEENPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables weapon sheen render in a second pass")
		VSHADER_PARAM(SHEENMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "sheenmap")
		VSHADER_PARAM(SHEENMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "sheenmap mask")
		VSHADER_PARAM(SHEENMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "")
		VSHADER_PARAM(SHEENMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "sheenmap tint")
		VSHADER_PARAM(SHEENMAPMASKSCALEX, SHADER_PARAM_TYPE_FLOAT, "1", "X Scale the size of the map mask to the size of the target")
		VSHADER_PARAM(SHEENMAPMASKSCALEY, SHADER_PARAM_TYPE_FLOAT, "1", "Y Scale the size of the map mask to the size of the target")
		VSHADER_PARAM(SHEENMAPMASKOFFSETX, SHADER_PARAM_TYPE_FLOAT, "0", "X Offset of the mask relative to model space coords of target")
		VSHADER_PARAM(SHEENMAPMASKOFFSETY, SHADER_PARAM_TYPE_FLOAT, "0", "Y Offset of the mask relative to model space coords of target")
		VSHADER_PARAM(SHEENMAPMASKDIRECTION, SHADER_PARAM_TYPE_INTEGER, "0", "The direction the sheen should move (length direction of weapon) XYZ, 0,1,2")
		VSHADER_PARAM(SHEENINDEX, SHADER_PARAM_TYPE_INTEGER, "0", "Index of the Effect Type (Color Additive, Override etc...)")

		// Flesh Interior Pass
		VSHADER_PARAM(FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass")
		VSHADER_PARAM(FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture")
		VSHADER_PARAM(FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture")
		VSHADER_PARAM(FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture")
		VSHADER_PARAM(FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture")
		VSHADER_PARAM(FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture")
		VSHADER_PARAM(FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture")
		VSHADER_PARAM(FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border")
		VSHADER_PARAM(FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh")
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		VSHADER_PARAM(FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color")
		VSHADER_PARAM(FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border")
		VSHADER_PARAM(FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)")
		VSHADER_PARAM(FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color")
		VSHADER_PARAM(FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity")
		VSHADER_PARAM(FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness")
		VSHADER_PARAM(FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed")

		VSHADER_PARAM(SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture")
		VSHADER_PARAM(LINEARWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of shader results.")
		VSHADER_PARAM(DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries. Only supported without bumpmaps")
		VSHADER_PARAM(DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges.")

		VSHADER_PARAM(BLENDTINTBYBASEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use the base alpha to blend in the $color modulation")
		VSHADER_PARAM(BLENDTINTCOLOROVERBASE, SHADER_PARAM_TYPE_FLOAT, "0", "blend between tint acting as a multiplication versus a replace");

		VSHADER_PARAM(VERTEXALPHATEST, SHADER_PARAM_TYPE_INTEGER, "0", "");

	};

	static constexpr size_t PARAM_COUNT = sizeof(XLitGenericParams) / sizeof(VulkanShaderParam);
	static_assert(sizeof(VulkanShaderParam[PARAM_COUNT]) == sizeof(XLitGenericParams));

	class XLitGeneric final : public IVulkanShader, public XLitGenericParams
	{
	public:
		XLitGeneric(const char* name) :
			IVulkanShader(GetParams(), PARAM_COUNT),
			m_Name(name)
		{
			assert(m_Name);
			GetShaderDLL()->InsertShader(this);
		}

		const char* GetName() const override;
		int GetFlags() const override;

		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
			const char* materialName) override;
		void OnDrawElements(IMaterialVar** params, IShaderShadow* shaderShadow,
			IShaderDynamicAPI* shaderAPI, VertexCompressionType_t vtxCompression,
			CBasePerMaterialContextData** context) override;

	private:
		void InitParamsVertexLitGeneric(IMaterialVar** params);
		void InitParamsCloakBlendedPass(IMaterialVar** params);
		void InitParamsWeaponSheenPass(IMaterialVar** params);
		void InitParamsEmissiveScrollBlendedPass(IMaterialVar** params);
		void InitParamsFleshInteriorBlendedPass(IMaterialVar** params);

		const char* m_Name;
	};
}

static XLitGeneric s_VertexLitGeneric("VertexLitGeneric");
static XLitGeneric s_UnlitGeneric("UnlitGeneric");

const char* XLitGeneric::GetName() const
{
	LOG_FUNC();
	return m_Name;
}

int XLitGeneric::GetFlags() const
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	std::vector<const char*> paramNames;
	for (size_t i = 0; params[i]; i++)
	{
		IMaterialVar& p = *params[i];
		paramNames.push_back(p.GetName());
	}

	InitParamsVertexLitGeneric(params);

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

void XLitGeneric::OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
	const char* materialName)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::OnDrawElements(IMaterialVar** params, IShaderShadow* shaderShadow,
	IShaderDynamicAPI* shaderAPI, VertexCompressionType_t vtxCompression,
	CBasePerMaterialContextData** context)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitParamsVertexLitGeneric(IMaterialVar** params)
{
	InitIntParam(PHONG, params, 0);

	InitFloatParam(ALPHATESTREFERENCE, params, 0.0f);
	InitIntParam(VERTEXALPHATEST, params, 0);

	InitIntParam(FLASHLIGHTNOLAMBERT, params, 0);

	if (info.m_nDetailTint != -1 && !params[info.m_nDetailTint]->IsDefined())
	{
		params[info.m_nDetailTint]->SetVecValue(1.0f, 1.0f, 1.0f);
	}

	if (info.m_nEnvmapTint != -1 && !params[info.m_nEnvmapTint]->IsDefined())
	{
		params[info.m_nEnvmapTint]->SetVecValue(1.0f, 1.0f, 1.0f);
	}

	InitIntParam(info.m_nEnvmapFrame, params, 0);
	InitIntParam(info.m_nBumpFrame, params, 0);
	InitFloatParam(info.m_nDetailTextureBlendFactor, params, 1.0);
	InitIntParam(info.m_nReceiveFlashlight, params, 0);

	InitFloatParam(info.m_nDetailScale, params, 4.0f);

	if ((info.m_nBlendTintByBaseAlpha != -1) && (!params[info.m_nBlendTintByBaseAlpha]->IsDefined()))
	{
		params[info.m_nBlendTintByBaseAlpha]->SetIntValue(0);
	}

	InitFloatParam(info.m_nTintReplacesBaseColor, params, 0);

	if ((info.m_nSelfIllumTint != -1) && (!params[info.m_nSelfIllumTint]->IsDefined()))
	{
		params[info.m_nSelfIllumTint]->SetVecValue(1.0f, 1.0f, 1.0f);
	}


	if (WantsSkinShader(params, info))
	{
		if (!g_pHardwareConfig->SupportsPixelShaders_2_b() || !g_pConfig->UsePhong())
		{
			params[info.m_nPhong]->SetIntValue(0);
		}
		else
		{
			InitParamsSkin_DX9(pShader, params, pMaterialName, info);
			return;
		}
	}

	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	if (info.m_nFlashlightTexture != -1)
	{
		if (g_pHardwareConfig->SupportsBorderColor())
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
		}
	}

	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if (info.m_nAlbedo != -1 && g_pConfig->UseBumpmapping() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined() && params[info.m_nAlbedo]->IsDefined() &&
		params[info.m_nBaseTexture]->IsDefined())
	{
		params[info.m_nBaseTexture]->SetStringValue(params[info.m_nAlbedo]->GetStringValue());
	}

	// This shader can be used with hw skinning
	SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);

	if (bVertexLitGeneric)
	{
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
	}
	else
	{
		CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
	}

	InitIntParam(info.m_nEnvmapMaskFrame, params, 0);
	InitFloatParam(info.m_nEnvmapContrast, params, 0.0);
	InitFloatParam(info.m_nEnvmapSaturation, params, 1.0f);
	InitFloatParam(info.m_nSeamlessScale, params, 0.0);

	// handle line art parms
	InitFloatParam(info.m_nEdgeSoftnessStart, params, 0.5);
	InitFloatParam(info.m_nEdgeSoftnessEnd, params, 0.5);
	InitFloatParam(info.m_nGlowAlpha, params, 1.0);
	InitFloatParam(info.m_nOutlineAlpha, params, 1.0);

	// No texture means no self-illum or env mask in base alpha
	if (info.m_nBaseTexture != -1 && !params[info.m_nBaseTexture]->IsDefined())
	{
		CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	}

	if (((info.m_nBumpmap != -1) && g_pConfig->UseBumpmapping() && params[info.m_nBumpmap]->IsDefined())
		// we don't need a tangent space if we have envmap without bumpmap
		//		|| ( info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() )
		)
	{
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}
	else if ((info.m_nDiffuseWarpTexture != -1) && params[info.m_nDiffuseWarpTexture]->IsDefined()) // diffuse warp goes down bump path...
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
		params[info.m_nEnvmapMask]->SetUndefined();
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) && info.m_nBumpmap != -1 &&
		params[info.m_nBumpmap]->IsDefined() && !hasNormalMapAlphaEnvmapMask)
	{
		Warning("material %s has a normal map and $basealphaenvmapmask.  Must use $normalmapalphaenvmapmask to get specular.\n\n", pMaterialName);
		params[info.m_nEnvmap]->SetUndefined();
	}

	if (info.m_nEnvmapMask != -1 && params[info.m_nEnvmapMask]->IsDefined() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined())
	{
		params[info.m_nEnvmapMask]->SetUndefined();
		if (!hasNormalMapAlphaEnvmapMask)
		{
			Warning("material %s has a normal map and an envmapmask.  Must use $normalmapalphaenvmapmask.\n\n", pMaterialName);
			params[info.m_nEnvmap]->SetUndefined();
		}
	}

	// If mat_specular 0, then get rid of envmap
	if (!g_pConfig->UseSpecular() && info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() && params[info.m_nBaseTexture]->IsDefined())
	{
		params[info.m_nEnvmap]->SetUndefined();
	}

	InitFloatParam(info.m_nHDRColorScale, params, 1.0f);

	InitIntParam(info.m_nLinearWrite, params, 0);
	InitIntParam(info.m_nGammaColorRead, params, 0);

	InitIntParam(info.m_nDepthBlend, params, 0);
	InitFloatParam(info.m_nDepthBlendScale, params, 50.0f);
}

void XLitGeneric::InitParamsCloakBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void XLitGeneric::InitParamsWeaponSheenPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void XLitGeneric::InitParamsEmissiveScrollBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void XLitGeneric::InitParamsFleshInteriorBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}
