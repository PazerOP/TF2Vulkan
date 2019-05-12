#pragma once

#include "VulkanShaderParam.h"

#include <TF2Vulkan/Util/SafeConvert.h>

namespace TF2Vulkan{ namespace Shaders
{
	struct BumpmapParams
	{
		VSHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map");
		VSHADER_PARAM(BUMPCOMPRESS, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "compression bump map");
		VSHADER_PARAM(BUMPSTRETCH, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "expansion bump map");
		VSHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap");
		VSHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform");
	};

	struct WrinkleParams
	{
		VSHADER_PARAM(COMPRESS, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "compression wrinklemap");
		VSHADER_PARAM(STRETCH, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "expansion wrinklemap");
	};

	struct EnvMapParams
	{
		VSHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap");
		VSHADER_PARAM(ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number");
		VSHADER_PARAM(ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask");
		VSHADER_PARAM(ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "");
		VSHADER_PARAM(ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform");
		VSHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint");
		VSHADER_PARAM(ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color");
		VSHADER_PARAM(ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal");
	};

	struct PhongParams
	{
		VSHADER_PARAM(PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting");
		VSHADER_PARAM(PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights");
		VSHADER_PARAM(PHONGTINT, SHADER_PARAM_TYPE_VEC3, "5.0", "Phong tint for local specular lights");
		VSHADER_PARAM(PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1.0", "Apply tint by albedo (controlled by spec exponent texture");
		VSHADER_PARAM(INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)");
		VSHADER_PARAM(BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha");
		VSHADER_PARAM(PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term");
		VSHADER_PARAM(PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0  0.5  1]", "Parameters for remapping fresnel output");
		VSHADER_PARAM(PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)");
		VSHADER_PARAM(PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map");
		VSHADER_PARAM(PHONGEXPONENTFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "When using a phong exponent texture, this will be multiplied by the 0..1 that comes out of the texture.");
	};

	struct RimlightParams
	{
		VSHADER_PARAM(RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting");
		VSHADER_PARAM(RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights");
		VSHADER_PARAM(RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights");
		VSHADER_PARAM(RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term");
	};

	struct SelfillumParams
	{
		VSHADER_PARAM(SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint");
		VSHADER_PARAM(SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "defines that self illum value comes from env map mask alpha");
		VSHADER_PARAM(SELFILLUMFRESNEL, SHADER_PARAM_TYPE_BOOL, "0", "Self illum fresnel");
		VSHADER_PARAM(SELFILLUMFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC4, "0", "Self illum fresnel min, max, exp");
		VSHADER_PARAM(SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum");
	};

	struct DetailParams
	{
		VSHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture");
		VSHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail");
		VSHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture");
		VSHADER_PARAM(DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade");
		VSHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.");
		VSHADER_PARAM(DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint");
		VSHADER_PARAM(DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform");
	};

	struct EmissiveScrollParams
	{
		VSHADER_PARAM(EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass");
		VSHADER_PARAM(EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map");
		VSHADER_PARAM(EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec");
		VSHADER_PARAM(EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength");
		VSHADER_PARAM(EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map");
		VSHADER_PARAM(EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint");
		VSHADER_PARAM(EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map");
		VSHADER_PARAM(TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy");
	};

	struct WeaponSheenParams
	{
		VSHADER_PARAM(SHEENPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables weapon sheen render in a second pass");
		VSHADER_PARAM(SHEENMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "sheenmap");
		VSHADER_PARAM(SHEENMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "sheenmap mask");
		VSHADER_PARAM(SHEENMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "");
		VSHADER_PARAM(SHEENMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "sheenmap tint");
		VSHADER_PARAM(SHEENMAPMASKSCALEX, SHADER_PARAM_TYPE_FLOAT, "1", "X Scale the size of the map mask to the size of the target");
		VSHADER_PARAM(SHEENMAPMASKSCALEY, SHADER_PARAM_TYPE_FLOAT, "1", "Y Scale the size of the map mask to the size of the target");
		VSHADER_PARAM(SHEENMAPMASKOFFSETX, SHADER_PARAM_TYPE_FLOAT, "0", "X Offset of the mask relative to model space coords of target");
		VSHADER_PARAM(SHEENMAPMASKOFFSETY, SHADER_PARAM_TYPE_FLOAT, "0", "Y Offset of the mask relative to model space coords of target");
		VSHADER_PARAM(SHEENMAPMASKDIRECTION, SHADER_PARAM_TYPE_INTEGER, "0", "The direction the sheen should move (length direction of weapon) XYZ, 0,1,2");
		VSHADER_PARAM(SHEENINDEX, SHADER_PARAM_TYPE_INTEGER, "0", "Index of the Effect Type (Color Additive, Override etc...)");
	};

	struct SeamlessScaleParams
	{
		VSHADER_PARAM(SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model.");
		VSHADER_PARAM(SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture.");
		VSHADER_PARAM(SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch.");
	};

	struct CloakParams
	{
		VSHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass");
		VSHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "");
		VSHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint");
		VSHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "");
	};

	struct FleshParams
	{
		VSHADER_PARAM(FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass");
		VSHADER_PARAM(FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture");
		VSHADER_PARAM(FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture");
		VSHADER_PARAM(FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture");
		VSHADER_PARAM(FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture");
		VSHADER_PARAM(FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture");
		VSHADER_PARAM(FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture");
		VSHADER_PARAM(FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border");
		VSHADER_PARAM(FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh");
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		VSHADER_PARAM(FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		VSHADER_PARAM(FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color");
		VSHADER_PARAM(FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border");
		VSHADER_PARAM(FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)");
		VSHADER_PARAM(FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color");
		VSHADER_PARAM(FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity");
		VSHADER_PARAM(FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness");
		VSHADER_PARAM(FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed");
	};

	struct DistanceAlphaParams
	{
		VSHADER_PARAM(SOFTEDGES, SHADER_PARAM_TYPE_BOOL, "0", "Enable soft edges to distance coded textures.");
		VSHADER_PARAM(SCALEEDGESOFTNESSBASEDONSCREENRES, SHADER_PARAM_TYPE_BOOL, "0", "Scale the size of the soft edges based upon resolution. 1024x768 = nominal.");
		VSHADER_PARAM(EDGESOFTNESSSTART, SHADER_PARAM_TYPE_FLOAT, "0.6", "Start value for soft edges for distancealpha.");
		VSHADER_PARAM(EDGESOFTNESSEND, SHADER_PARAM_TYPE_FLOAT, "0.5", "End value for soft edges for distancealpha.");

		VSHADER_PARAM(GLOW, SHADER_PARAM_TYPE_BOOL, "0", "Enable glow/shadow for distance coded textures.");
		VSHADER_PARAM(GLOWCOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "color of outter glow for distance coded line art.");
		VSHADER_PARAM(GLOWALPHA, SHADER_PARAM_TYPE_FLOAT, "1", "Base glow alpha amount for glows/shadows with distance alpha.");
		VSHADER_PARAM(GLOWSTART, SHADER_PARAM_TYPE_FLOAT, "0.7", "start value for glow/shadow");
		VSHADER_PARAM(GLOWEND, SHADER_PARAM_TYPE_FLOAT, "0.5", "end value for glow/shadow");
		VSHADER_PARAM(GLOWX, SHADER_PARAM_TYPE_FLOAT, "0", "texture offset x for glow mask.");
		VSHADER_PARAM(GLOWY, SHADER_PARAM_TYPE_FLOAT, "0", "texture offset y for glow mask.");

		VSHADER_PARAM(OUTLINE, SHADER_PARAM_TYPE_BOOL, "0", "Enable outline for distance coded textures.");
		VSHADER_PARAM(OUTLINECOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "color of outline for distance coded images.");
		VSHADER_PARAM(OUTLINEALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "alpha value for outline");
		VSHADER_PARAM(OUTLINESTART0, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer start value for outline");
		VSHADER_PARAM(OUTLINESTART1, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner start value for outline");
		VSHADER_PARAM(OUTLINEEND0, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner end value for outline");
		VSHADER_PARAM(OUTLINEEND1, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer end value for outline");
		VSHADER_PARAM(SCALEOUTLINESOFTNESSBASEDONSCREENRES, SHADER_PARAM_TYPE_BOOL, "0", "Scale the size of the soft part of the outline based upon resolution. 1024x768 = nominal.");
	};

	template<typename T>
	static constexpr size_t GetShaderParamCount()
	{
		constexpr size_t PARAM_COUNT = sizeof(T) / sizeof(VulkanShaderParam);
		static_assert(sizeof(VulkanShaderParam[PARAM_COUNT]) == sizeof(T));
		return PARAM_COUNT;
	}

	template<typename T>
	inline static const VulkanShaderParam* GetAsShaderParams(const T& type)
	{
		// Make sure these static_asserts get evalutated
		GetShaderParamCount<T>();

		return reinterpret_cast<const VulkanShaderParam*>(&type);
	}

	template<typename T>
	inline static VulkanShaderParam* GetAsShaderParams(T& type)
	{
		return const_cast<VulkanShaderParam*>(GetAsShaderParams(const_cast<const T&>(type)));
	}

	template<typename T>
	inline void InitParamIndices(T& type)
	{
		auto params = GetAsShaderParams(type);
		for (size_t i = 0; i < GetShaderParamCount<T>(); i++)
			params[i].InitIndex(Util::SafeConvert<int>(i));
	}
} }
