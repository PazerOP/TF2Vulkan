#include "BaseShaderNext.h"
#include "ParamGroups.h"

#include <TF2Vulkan/AlignedTypes.h>
#include <TF2Vulkan/ISpecConstLayout.h>
#include <TF2Vulkan/Util/std_array.h>
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

	class XLitGeneric : public ShaderNext<XLitGeneric, Params>
	{
	public:
		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
			const char* materialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;

		virtual bool IsVertexLitGeneric() const { return false; }

	private:
		bool WantsSkinShader(IMaterialVar** params) const;

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

		void DrawVertexLitGeneric(const OnDrawElementsParams& params);
		void DrawWeaponSheenPass(const OnDrawElementsParams& params);
		void DrawCloakBlendedPass(const OnDrawElementsParams& params);
		void DrawEmissiveScrollBlendedPass(const OnDrawElementsParams& params);
		void DrawFleshInteriorBlendedPass(const OnDrawElementsParams& params);

		bool CloakBlendedPassIsFullyOpaque(IMaterialVar** params) const;
		bool ShouldDrawMaterialSheen(IMaterialVar** params) const;
	};

	class UnlitGeneric final : public XLitGeneric
	{
	public:
		const char* GetName() const override { return "UnlitGeneric"; }
	};

	class VertexLitGeneric final : public XLitGeneric
	{
	public:
		const char* GetName() const override { return "VertexLitGeneric"; }

		bool IsVertexLitGeneric() const override { return true; }
	};

	class Wireframe final : public XLitGeneric
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
//DEFINE_NSHADER_FALLBACK(VertexLitGeneric_DX8, VertexLitGeneric);

void Wireframe::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	SET_FLAGS(MATERIAL_VAR_NOFOG);
	SET_FLAGS(MATERIAL_VAR_WIREFRAME);

	XLitGeneric::OnInitShaderParams(params, materialName);
}

void XLitGeneric::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
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

void XLitGeneric::OnInitShaderInstance(IMaterialVar** params, IShaderInit* shaderInit,
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

bool XLitGeneric::CloakBlendedPassIsFullyOpaque(IMaterialVar** params) const
{
	// TODO: Figure out if this is more complicated
	return params[CLOAKFACTOR]->GetFloatValue() >= 0.9995f;
}

bool XLitGeneric::ShouldDrawMaterialSheen(IMaterialVar** params) const
{
	// TODO: Is this more complicated?
	return !!params[SHEENPASSENABLED]->GetIntValue();
}

void XLitGeneric::DrawVertexLitGeneric(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();

	auto& shadow = params.shadow;
	auto& dynamic = params.dynamic;
	if (shadow)
	{
		NOT_IMPLEMENTED_FUNC();
		//shadow->SetPixelShader("xlitgeneric_ps");
		//shadow->SetVertexShader("xlitgeneric_vs");
	}

	if (dynamic)
	{

	}

	Draw();
}

void XLitGeneric::DrawWeaponSheenPass(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::DrawCloakBlendedPass(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::DrawEmissiveScrollBlendedPass(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::DrawFleshInteriorBlendedPass(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::OnDrawElements(const OnDrawElementsParams& params)
{
	// Skip the standard rendering if cloak pass is fully opaque
	bool bDrawStandardPass = true;
	if (params[CLOAKPASSENABLED]->GetIntValue() && (params.shadow == NULL)) // && not snapshotting
	{
		if (CloakBlendedPassIsFullyOpaque(params.matvars))
			bDrawStandardPass = false;
	}

	// Standard rendering pass
	if (bDrawStandardPass)
	{
		DrawVertexLitGeneric(params);
	}
	else
	{
		// Skip this pass!
		Draw(false);
	}

	// Weapon sheen pass
	// only if doing standard as well (don't do it if cloaked)
	if (params[SHEENPASSENABLED]->GetIntValue())
	{
		if ((params.shadow != NULL) || (bDrawStandardPass && ShouldDrawMaterialSheen(params.matvars)))
		{
			DrawWeaponSheenPass(params);
		}
		else
		{
			// Skip this pass!
			Draw(false);
		}
	}

	// Cloak Pass
	if (params[CLOAKPASSENABLED]->GetIntValue())
	{
		// If ( snapshotting ) or ( we need to draw this frame )
		if ((params.shadow != NULL) || ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)))
		{
			DrawCloakBlendedPass(params);
		}
		else // We're not snapshotting and we don't need to draw this frame
		{
			// Skip this pass!
			Draw(false);
		}
	}

	// Emissive Scroll Pass
	if (params[EMISSIVEBLENDENABLED]->GetIntValue())
	{
		// If ( snapshotting ) or ( we need to draw this frame )
		if ((params.shadow != NULL) || (params[EMISSIVEBLENDSTRENGTH]->GetFloatValue() > 0.0f))
		{
			DrawEmissiveScrollBlendedPass(params);
		}
		else // We're not snapshotting and we don't need to draw this frame
		{
			// Skip this pass!
			Draw(false);
		}
	}

	// Flesh Interior Pass
	if (params[FLESHINTERIORENABLED]->GetIntValue())
	{
		// If ( snapshotting ) or ( we need to draw this frame )
		if ((params.shadow != NULL) || (true))
		{
			DrawFleshInteriorBlendedPass(params);
		}
		else // We're not snapshotting and we don't need to draw this frame
		{
			// Skip this pass!
			Draw(false);
		}
	}
}

bool XLitGeneric::WantsSkinShader(IMaterialVar** params) const
{
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

void XLitGeneric::InitParamsVertexLitGeneric(IMaterialVar** params, const char* materialName) const
{
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

void XLitGeneric::InitParamsSkin(IMaterialVar** params) const
{
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

void XLitGeneric::InitParamsCloakBlendedPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitParamsWeaponSheenPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitParamsEmissiveScrollBlendedPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitParamsFleshInteriorBlendedPass(IMaterialVar** params) const
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitShaderSkin(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitShaderVertexLitGeneric(IMaterialVar** params)
{
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

void XLitGeneric::InitShaderCloakBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitShaderWeaponSheenPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitShaderEmissiveScrollBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}

void XLitGeneric::InitShaderFleshInteriorBlendedPass(IMaterialVar** params)
{
	NOT_IMPLEMENTED_FUNC();
}
