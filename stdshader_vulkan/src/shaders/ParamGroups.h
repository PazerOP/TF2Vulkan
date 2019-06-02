#pragma once

#include "ShaderComponents/BaseShaderComponent.h"
#include "ShaderParamNext.h"
#include "shaders/UniformBufConstructs/TextureTransform.h"
#include "TF2Vulkan/ISpecConstLayout.h"

#include <TF2Vulkan/Util/SafeConvert.h>

#include <shaderlib/cshader.h>

namespace TF2Vulkan{ namespace Shaders
{
	struct WrinkleParams
	{
		NSHADER_PARAM(COMPRESS, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "compression wrinklemap");
		NSHADER_PARAM(STRETCH, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "expansion wrinklemap");

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
	};

	struct DepthBlendParams
	{
		NSHADER_PARAM(DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries.");
		NSHADER_PARAM(DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges.");

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
	};

	struct EnvMapUniforms
	{
		TextureTransform m_EnvMapMaskTransform;
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
		NSHADER_PARAM(ENVMAPFRESNEL, SHADER_PARAM_TYPE_FLOAT, "0", "Degree to which Fresnel should be applied to env map");

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const
		{
			if (IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK))
			{
				params[ENVMAPMASK]->SetUndefined();
				CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			}
		}
		void PreDraw(IMaterialVar** params, EnvMapUniforms* uniformBuf, void* specConstBuf) const
		{
			uniformBuf->m_EnvMapMaskTransform = TextureTransform(params[ENVMAPMASKTRANSFORM]->GetMatrixValue());
		}
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

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
	};

	struct SeamlessScaleParams
	{
		NSHADER_PARAM(SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model.");
		NSHADER_PARAM(SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture.");
		NSHADER_PARAM(SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch.");

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
	};

	struct CloakParams
	{
		NSHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass");
		NSHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "");
		NSHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1 1]", "Cloak color tint");
		NSHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "");

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
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

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
	};

	struct DistanceAlphaParams
	{
		NSHADER_PARAM(DISTANCEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use distance-coded alpha generated from hi-res texture by vtex.");
		NSHADER_PARAM(DISTANCEALPHAFROMDETAIL, SHADER_PARAM_TYPE_BOOL, "0", "Take the distance-coded alpha mask from the detail texture.");

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

	private:
		template<typename... TGroups> friend class ShaderComponents;
		void InitParamGroup(IMaterialVar** params) const {}
		void PreDraw(IMaterialVar** params, void* uniformBuf, void* specConstBuf) const {}
	};
} }
