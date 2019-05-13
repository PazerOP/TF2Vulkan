#pragma once

#include "ShaderParamNext.h"

#include <TF2Vulkan/Util/SafeConvert.h>

namespace TF2Vulkan{ namespace Shaders
{
	struct BumpmapParams
	{
		NSHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map");
		NSHADER_PARAM(BUMPCOMPRESS, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "compression bump map");
		NSHADER_PARAM(BUMPSTRETCH, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "expansion bump map");
		NSHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap");
		NSHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform");
	};

	struct WrinkleParams
	{
		NSHADER_PARAM(COMPRESS, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "compression wrinklemap");
		NSHADER_PARAM(STRETCH, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "expansion wrinklemap");
	};

	struct EnvMapParams
	{
		NSHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap");
		NSHADER_PARAM(ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number");
		NSHADER_PARAM(ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask");
		NSHADER_PARAM(ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "");
		NSHADER_PARAM(ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform");
		NSHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint");
		NSHADER_PARAM(ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color");
		NSHADER_PARAM(ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal");
	};

	struct PhongParams
	{
		NSHADER_PARAM(PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting");
		NSHADER_PARAM(PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights");
		NSHADER_PARAM(PHONGTINT, SHADER_PARAM_TYPE_VEC3, "5.0", "Phong tint for local specular lights");
		NSHADER_PARAM(PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1.0", "Apply tint by albedo (controlled by spec exponent texture");
		NSHADER_PARAM(INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)");
		NSHADER_PARAM(BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha");
		NSHADER_PARAM(PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term");
		NSHADER_PARAM(PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0  0.5  1]", "Parameters for remapping fresnel output");
		NSHADER_PARAM(PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)");
		NSHADER_PARAM(PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map");
		NSHADER_PARAM(PHONGEXPONENTFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "When using a phong exponent texture, this will be multiplied by the 0..1 that comes out of the texture.");
	};

	struct RimlightParams
	{
		NSHADER_PARAM(RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting");
		NSHADER_PARAM(RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights");
		NSHADER_PARAM(RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights");
		NSHADER_PARAM(RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term");
	};

	struct SelfillumParams
	{
		NSHADER_PARAM(SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint");
		NSHADER_PARAM(SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "defines that self illum value comes from env map mask alpha");
		NSHADER_PARAM(SELFILLUMFRESNEL, SHADER_PARAM_TYPE_BOOL, "0", "Self illum fresnel");
		NSHADER_PARAM(SELFILLUMFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC4, "0", "Self illum fresnel min, max, exp");
		NSHADER_PARAM(SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum");
	};

	struct DetailParams
	{
		NSHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture");
		NSHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail");
		NSHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture");
		NSHADER_PARAM(DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade");
		NSHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture.");
		NSHADER_PARAM(DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint");
		NSHADER_PARAM(DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform");
	};

	struct EmissiveScrollParams
	{
		NSHADER_PARAM(EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass");
		NSHADER_PARAM(EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map");
		NSHADER_PARAM(EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec");
		NSHADER_PARAM(EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength");
		NSHADER_PARAM(EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map");
		NSHADER_PARAM(EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint");
		NSHADER_PARAM(EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map");
		NSHADER_PARAM(TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy");
	};

	struct WeaponSheenParams
	{
		NSHADER_PARAM(SHEENPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables weapon sheen render in a second pass");
		NSHADER_PARAM(SHEENMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "sheenmap");
		NSHADER_PARAM(SHEENMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "sheenmap mask");
		NSHADER_PARAM(SHEENMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "");
		NSHADER_PARAM(SHEENMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "sheenmap tint");
		NSHADER_PARAM(SHEENMAPMASKSCALEX, SHADER_PARAM_TYPE_FLOAT, "1", "X Scale the size of the map mask to the size of the target");
		NSHADER_PARAM(SHEENMAPMASKSCALEY, SHADER_PARAM_TYPE_FLOAT, "1", "Y Scale the size of the map mask to the size of the target");
		NSHADER_PARAM(SHEENMAPMASKOFFSETX, SHADER_PARAM_TYPE_FLOAT, "0", "X Offset of the mask relative to model space coords of target");
		NSHADER_PARAM(SHEENMAPMASKOFFSETY, SHADER_PARAM_TYPE_FLOAT, "0", "Y Offset of the mask relative to model space coords of target");
		NSHADER_PARAM(SHEENMAPMASKDIRECTION, SHADER_PARAM_TYPE_INTEGER, "0", "The direction the sheen should move (length direction of weapon) XYZ, 0,1,2");
		NSHADER_PARAM(SHEENINDEX, SHADER_PARAM_TYPE_INTEGER, "0", "Index of the Effect Type (Color Additive, Override etc...)");
	};

	struct SeamlessScaleParams
	{
		NSHADER_PARAM(SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model.");
		NSHADER_PARAM(SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture.");
		NSHADER_PARAM(SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch.");
	};

	struct CloakParams
	{
		NSHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass");
		NSHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "");
		NSHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint");
		NSHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "");
	};

	struct FleshParams
	{
		NSHADER_PARAM(FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass");
		NSHADER_PARAM(FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture");
		NSHADER_PARAM(FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture");
		NSHADER_PARAM(FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture");
		NSHADER_PARAM(FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture");
		NSHADER_PARAM(FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture");
		NSHADER_PARAM(FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture");
		NSHADER_PARAM(FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border");
		NSHADER_PARAM(FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh");
		NSHADER_PARAM(FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		NSHADER_PARAM(FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		NSHADER_PARAM(FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		NSHADER_PARAM(FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius");
		NSHADER_PARAM(FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color");
		NSHADER_PARAM(FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border");
		NSHADER_PARAM(FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)");
		NSHADER_PARAM(FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color");
		NSHADER_PARAM(FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity");
		NSHADER_PARAM(FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness");
		NSHADER_PARAM(FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed");
	};

	struct DistanceAlphaParams
	{
		NSHADER_PARAM(SOFTEDGES, SHADER_PARAM_TYPE_BOOL, "0", "Enable soft edges to distance coded textures.");
		NSHADER_PARAM(SCALEEDGESOFTNESSBASEDONSCREENRES, SHADER_PARAM_TYPE_BOOL, "0", "Scale the size of the soft edges based upon resolution. 1024x768 = nominal.");
		NSHADER_PARAM(EDGESOFTNESSSTART, SHADER_PARAM_TYPE_FLOAT, "0.6", "Start value for soft edges for distancealpha.");
		NSHADER_PARAM(EDGESOFTNESSEND, SHADER_PARAM_TYPE_FLOAT, "0.5", "End value for soft edges for distancealpha.");

		NSHADER_PARAM(GLOW, SHADER_PARAM_TYPE_BOOL, "0", "Enable glow/shadow for distance coded textures.");
		NSHADER_PARAM(GLOWCOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "color of outter glow for distance coded line art.");
		NSHADER_PARAM(GLOWALPHA, SHADER_PARAM_TYPE_FLOAT, "1", "Base glow alpha amount for glows/shadows with distance alpha.");
		NSHADER_PARAM(GLOWSTART, SHADER_PARAM_TYPE_FLOAT, "0.7", "start value for glow/shadow");
		NSHADER_PARAM(GLOWEND, SHADER_PARAM_TYPE_FLOAT, "0.5", "end value for glow/shadow");
		NSHADER_PARAM(GLOWX, SHADER_PARAM_TYPE_FLOAT, "0", "texture offset x for glow mask.");
		NSHADER_PARAM(GLOWY, SHADER_PARAM_TYPE_FLOAT, "0", "texture offset y for glow mask.");

		NSHADER_PARAM(OUTLINE, SHADER_PARAM_TYPE_BOOL, "0", "Enable outline for distance coded textures.");
		NSHADER_PARAM(OUTLINECOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "color of outline for distance coded images.");
		NSHADER_PARAM(OUTLINEALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "alpha value for outline");
		NSHADER_PARAM(OUTLINESTART0, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer start value for outline");
		NSHADER_PARAM(OUTLINESTART1, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner start value for outline");
		NSHADER_PARAM(OUTLINEEND0, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner end value for outline");
		NSHADER_PARAM(OUTLINEEND1, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer end value for outline");
		NSHADER_PARAM(SCALEOUTLINESOFTNESSBASEDONSCREENRES, SHADER_PARAM_TYPE_BOOL, "0", "Scale the size of the soft part of the outline based upon resolution. 1024x768 = nominal.");
	};

	template<typename T>
	static constexpr size_t GetShaderParamCount()
	{
		constexpr size_t PARAM_COUNT = sizeof(T) / sizeof(ShaderParamNext);
		static_assert(sizeof(ShaderParamNext[PARAM_COUNT]) == sizeof(T));
		return PARAM_COUNT;
	}

	template<typename T>
	inline static const ShaderParamNext* GetAsShaderParams(const T& type)
	{
		// Make sure these static_asserts get evalutated
		GetShaderParamCount<T>();

		return reinterpret_cast<const ShaderParamNext*>(&type);
	}

	template<typename T>
	inline static ShaderParamNext* GetAsShaderParams(T& type)
	{
		return const_cast<ShaderParamNext*>(GetAsShaderParams(const_cast<const T&>(type)));
	}

	template<typename T>
	inline T& InitParamIndices(T& type)
	{
		auto params = GetAsShaderParams(type);
		for (size_t i = 0; i < GetShaderParamCount<T>(); i++)
			params[i].InitIndex(Util::SafeConvert<int>(i) + NUM_SHADER_MATERIAL_VARS);

		return type;
	}
} }
