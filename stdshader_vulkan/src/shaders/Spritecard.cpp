#include "BaseShaderNext.h"
#include "ParamGroups.h"

#include <TF2Vulkan/IShaderNextFactory.h>
#include <TF2Vulkan/Util/Macros.h>

#include <tier0/icommandline.h>

using namespace TF2Vulkan;
using namespace TF2Vulkan::Shaders;

#define SHADER_NAME Spritecard

#define DEFAULT_PARTICLE_FEATHERING_ENABLED 1

inline namespace SHADER_NAME
{
	static int GetDefaultDepthFeatheringValue(void) //Allow the command-line to go against the default soft-particle value
	{
		static int iRetVal = -1;

		if (iRetVal == -1)
		{
#if ( DEFAULT_PARTICLE_FEATHERING_ENABLED == 1 )
			{
				if (CommandLine()->CheckParm("-softparticlesdefaultoff"))
					iRetVal = 0;
				else
					iRetVal = 1;
			}
#else
			{
				if (CommandLine()->CheckParm("-softparticlesdefaulton"))
					iRetVal = 1;
				else
					iRetVal = 0;
			}
#endif
		}

		return iRetVal;
	}

	struct Params : DistanceAlphaParams
	{
		NSHADER_PARAM(DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries");
		NSHADER_PARAM(DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges.");
		NSHADER_PARAM(INVERSEDEPTHBLEND, SHADER_PARAM_TYPE_BOOL, "0", "calculate 1-depthblendalpha so that sprites appear when they are near geometry");
		NSHADER_PARAM(ORIENTATION, SHADER_PARAM_TYPE_INTEGER, "0", "0 = always face camera, 1 = rotate around z, 2= parallel to ground 3=use normal");
		NSHADER_PARAM(ADDBASETEXTURE2, SHADER_PARAM_TYPE_FLOAT, "0.0", "amount to blend second texture into frame by");
		NSHADER_PARAM(OVERBRIGHTFACTOR, SHADER_PARAM_TYPE_FLOAT, "1.0", "overbright factor for texture. For HDR effects.");
		NSHADER_PARAM(DUALSEQUENCE, SHADER_PARAM_TYPE_INTEGER, "0", "blend two separate animated sequences.");
		NSHADER_PARAM(SEQUENCE_BLEND_MODE, SHADER_PARAM_TYPE_INTEGER, "0", "defines the blend mode between the images un dual sequence particles. 0 = avg, 1=alpha from first, rgb from 2nd, 2= first over second");
		NSHADER_PARAM(MAXLUMFRAMEBLEND1, SHADER_PARAM_TYPE_INTEGER, "0", "instead of blending between animation frames for the first sequence, select pixels based upon max luminance");
		NSHADER_PARAM(MAXLUMFRAMEBLEND2, SHADER_PARAM_TYPE_INTEGER, "0", "instead of blending between animation frames for the 2nd sequence, select pixels based upon max luminance");
		NSHADER_PARAM(RAMPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "if specified, then the red value of the image is used to index this ramp to produce the output color");
		NSHADER_PARAM(ZOOMANIMATESEQ2, SHADER_PARAM_TYPE_FLOAT, "1.0", "amount to gradually zoom between frames on the second sequence. 2.0 will double the size of a frame over its lifetime.");
		NSHADER_PARAM(EXTRACTGREENALPHA, SHADER_PARAM_TYPE_INTEGER, "0", "grayscale data sitting in green/alpha channels");
		NSHADER_PARAM(ADDOVERBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "use ONE:INVSRCALPHA blending");
		NSHADER_PARAM(ADDSELF, SHADER_PARAM_TYPE_FLOAT, "0.0", "amount of base texture to additively blend in");
		NSHADER_PARAM(BLENDFRAMES, SHADER_PARAM_TYPE_BOOL, "1", "whether or not to smoothly blend between animated frames");
		NSHADER_PARAM(MINSIZE, SHADER_PARAM_TYPE_FLOAT, "0.0", "minimum screen fractional size of particle");
		NSHADER_PARAM(STARTFADESIZE, SHADER_PARAM_TYPE_FLOAT, "10.0", "screen fractional size to start fading particle out");
		NSHADER_PARAM(ENDFADESIZE, SHADER_PARAM_TYPE_FLOAT, "20.0", "screen fractional size to finish fading particle out");
		NSHADER_PARAM(MAXSIZE, SHADER_PARAM_TYPE_FLOAT, "20.0", "maximum screen fractional size of particle");
		NSHADER_PARAM(USEINSTANCING, SHADER_PARAM_TYPE_BOOL, "1", "whether to use GPU vertex instancing (submit 1 vert per particle quad)");
		NSHADER_PARAM(SPLINETYPE, SHADER_PARAM_TYPE_INTEGER, "0", "spline type 0 = none,  1=ctamull rom");
		NSHADER_PARAM(MAXDISTANCE, SHADER_PARAM_TYPE_FLOAT, "100000.0", "maximum distance to draw particles at");
		NSHADER_PARAM(FARFADEINTERVAL, SHADER_PARAM_TYPE_FLOAT, "400.0", "interval over which to fade out far away particles");
		NSHADER_PARAM(SHADERSRGBREAD360, SHADER_PARAM_TYPE_BOOL, "0", "Simulate srgb read in shader code");
		NSHADER_PARAM(ORIENTATIONMATRIX, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "Matrix used to orient in orientation mode #2");
		NSHADER_PARAM(MOD2X, SHADER_PARAM_TYPE_BOOL, "0", "whether or not to multiply the result of the pixel shader * 2 against the framebuffer");
		NSHADER_PARAM(ALPHATRAILFADE, SHADER_PARAM_TYPE_FLOAT, "1", "Amount to scale alpha by between start and end of trail/rope");
		NSHADER_PARAM(RADIUSTRAILFADE, SHADER_PARAM_TYPE_FLOAT, "1", "Amount to scale radis by between start and end of trail/rope");
		NSHADER_PARAM(SHADOWDEPTH, SHADER_PARAM_TYPE_INTEGER, "0", "writing to a shadow depth buffer");
		NSHADER_PARAM(OPAQUE, SHADER_PARAM_TYPE_BOOL, "0", "Are we opaque? (defaults to 0)");
		NSHADER_PARAM(CROPFACTOR, SHADER_PARAM_TYPE_VEC2, "[1 1]", "writing to a shadow depth buffer");
		NSHADER_PARAM(VERTEXCOLORLERP, SHADER_PARAM_TYPE_BOOL, "0", "Enable computing vertex color by interpolating two color based on vertex r color channel");
		NSHADER_PARAM(LERPCOLOR1, SHADER_PARAM_TYPE_VEC3, "[1 0 0]", "Lerp color 1");
		NSHADER_PARAM(LERPCOLOR2, SHADER_PARAM_TYPE_VEC3, "[0 1 0]", "Lerp color 2");
		NSHADER_PARAM(VERTEXFOGAMOUNT, SHADER_PARAM_TYPE_FLOAT, "1", "Amount of vertex fog to apply");

		// distance outline control
		NSHADER_PARAM(DISTANCEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use distance-coded alpha generated from hi-res texture by vtex.");

		NSHADER_PARAM(SOFTEDGES, SHADER_PARAM_TYPE_BOOL, "0", "Enable soft edges to distance coded textures.");
		NSHADER_PARAM(EDGESOFTNESSSTART, SHADER_PARAM_TYPE_FLOAT, "0.6", "Start value for soft edges for distancealpha.");
		NSHADER_PARAM(EDGESOFTNESSEND, SHADER_PARAM_TYPE_FLOAT, "0.5", "End value for soft edges for distancealpha.");

		NSHADER_PARAM(OUTLINE, SHADER_PARAM_TYPE_BOOL, "0", "Enable outline for distance coded textures.");
		NSHADER_PARAM(OUTLINECOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "color of outline for distance coded images.");
		NSHADER_PARAM(OUTLINEALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "alpha value for outline");
		NSHADER_PARAM(OUTLINESTART0, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer start value for outline");
		NSHADER_PARAM(OUTLINESTART1, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner start value for outline");
		NSHADER_PARAM(OUTLINEEND0, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner end value for outline");
		NSHADER_PARAM(OUTLINEEND1, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer end value for outline");
		NSHADER_PARAM(PERPARTICLEOUTLINE, SHADER_PARAM_TYPE_BOOL, "0", "Allow per particle outline control");
	};

	class Shader final : public ShaderNext<Shader, Params>
	{
	public:
		const char* GetName() const override { return V_STRINGIFY(SHADER_NAME); }

		void OnInitShaderParams(IMaterialVar** params, const char* materialName) override;
		void OnInitShader(IShaderNextFactory& factory) override;

		void OnInitShaderInstance(IMaterialVar** ppParams, IShaderInit* pShaderInit, const char* pMaterialName) override;
		void OnDrawElements(const OnDrawElementsParams& params) override;

	private:
		IShaderGroup* m_VSShader = nullptr;
		IShaderGroup* m_PSShader = nullptr;
	};

	static const Shader::InstanceRegister EXPAND_CONCAT(s_, SHADER_NAME);
}

void Shader::OnInitShader(IShaderNextFactory& factory)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	//m_VSShader = &factory.FindOrCreateShaderGroup(ShaderType::Vertex, "spritecard_vs");
	//m_PSShader = &factory.FindOrCreateShaderGroup(ShaderType::Pixel, "spritecard_ps");
}

void Shader::OnInitShaderInstance(IMaterialVar** params, IShaderInit* init, const char* materialName)
{
	SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);

	LoadTexture(BASETEXTURE);
	LoadTexture(RAMPTEXTURE);
}

void Shader::OnDrawElements(const OnDrawElementsParams& params)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
}

void Shader::OnInitShaderParams(IMaterialVar** params, const char* materialName)
{
	InitFloatParam(MAXDISTANCE, params, 100000);
	InitFloatParam(FARFADEINTERVAL, params, 400);
	InitFloatParam(MAXSIZE, params, 20);
	InitFloatParam(ENDFADESIZE, params, 20.0);
	InitFloatParam(STARTFADESIZE, params, 10.0);
	InitFloatParam(DEPTHBLENDSCALE, params, 50.0);
	InitFloatParam(OVERBRIGHTFACTOR, params, 1.0);
	InitFloatParam(ADDBASETEXTURE2, params, 0.0);
	InitFloatParam(ADDSELF, params, 0.0);
	InitFloatParam(ZOOMANIMATESEQ2, params, 0.0);
	InitFloatParam(ALPHATRAILFADE, params, 1.);
	InitFloatParam(RADIUSTRAILFADE, params, 1.);
	InitFloatParam(VERTEXFOGAMOUNT, params, 0.0f);
	InitFloatParam(OUTLINEALPHA, params, 1.0);

	if (!params[ORIENTATIONMATRIX]->IsDefined())
	{
		VMatrix mat;
		MatrixSetIdentity(mat);
		params[ORIENTATIONMATRIX]->SetMatrixValue(mat);
	}

	InitVecParam(CROPFACTOR, params, 1.0f, 1.0f);

	InitIntParam(DEPTHBLEND, params, GetDefaultDepthFeatheringValue());

	InitIntParam(DUALSEQUENCE, params, 0);
	InitIntParam(MAXLUMFRAMEBLEND1, params, 0);
	InitIntParam(MAXLUMFRAMEBLEND2, params, 0);
	InitIntParam(EXTRACTGREENALPHA, params, 0);
	InitIntParam(ADDOVERBLEND, params, 0);
	InitIntParam(BLENDFRAMES, params, 1);

	InitIntParam(DISTANCEALPHA, params, 0);
	InitIntParam(OUTLINE, params, 0);
	InitIntParam(SOFTEDGES, params, 0);
	InitIntParam(PERPARTICLEOUTLINE, params, 0);

	InitIntParam(USEINSTANCING, params, IsX360());

	// srgb read 360
	InitIntParam(SHADERSRGBREAD360, params, 0);

	// default to being translucent since that's what we always were for historical reasons.
	InitIntParam(OPAQUE, params, 0);

	InitIntParam(VERTEXCOLORLERP, params, 0);
	InitVecParam(LERPCOLOR1, params, 1, 0, 0);
	InitVecParam(LERPCOLOR2, params, 0, 1, 0);

	if (params[OPAQUE]->GetBoolValue())
	{
		// none of these make sense if we have $opaque 1:
		params[ADDBASETEXTURE2]->SetFloatValue(0.0f);
		params[DUALSEQUENCE]->SetIntValue(0);
		params[SEQUENCE_BLEND_MODE]->SetIntValue(0);
		params[MAXLUMFRAMEBLEND1]->SetIntValue(0);
		params[MAXLUMFRAMEBLEND2]->SetIntValue(0);
		params[EXTRACTGREENALPHA]->SetIntValue(0);
		params[RAMPTEXTURE]->SetUndefined();
		params[ZOOMANIMATESEQ2]->SetIntValue(0);
		params[ADDOVERBLEND]->SetIntValue(0);
		params[ADDSELF]->SetIntValue(0);
		params[BLENDFRAMES]->SetIntValue(0);
		params[DEPTHBLEND]->SetIntValue(0);
		params[INVERSEDEPTHBLEND]->SetIntValue(0);
	}

	SET_FLAGS2(MATERIAL_VAR2_IS_SPRITECARD);
}
