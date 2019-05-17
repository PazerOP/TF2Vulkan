//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=====================================================================================//

#include "IShaderSystem.h"
#include "ShaderDLL.h"

#include <TF2Vulkan/Util/Macros.h>
#include <TF2Vulkan/Util/SafeConvert.h>
#include <TF2Vulkan/Util/ValuePusher.h>

#include <materialsystem/imaterial.h>
#include <shaderapi/ishaderdynamic.h>
#include <shaderlib/BaseShader.h>
#include <shaderlib/cshader.h>
#include <renderparm.h>

#include <iterator>

// TODO: Make thread_local or eliminate
IMaterialVar** CBaseShader::s_ppParams;
const char* CBaseShader::s_pTextureGroupName;
IShaderShadow* CBaseShader::s_pShaderShadow;
IShaderDynamicAPI* CBaseShader::s_pShaderAPI;
IShaderInit* CBaseShader::s_pShaderInit;

int CBaseShader::s_nModulationFlags;
CMeshBuilder* CBaseShader::s_pMeshBuilder;

static const ShaderParamInfo_t s_StandardParams[NUM_SHADER_MATERIAL_VARS] =
{
	{ "$flags", "flags", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags_defined", "flags_defined", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags2", "flags2", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$flags_defined2", "flags2_defined", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$color", "color", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", 0 },
	{ "$alpha", "alpha", SHADER_PARAM_TYPE_FLOAT, "1.0", 0 },
	{ "$basetexture", "Base Texture with lighting built in", SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", 0 },
	{ "$frame", "Animation Frame", SHADER_PARAM_TYPE_INTEGER, "0", 0 },
	{ "$basetexturetransform", "Base Texture Texcoord Transform", SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", 0 },
	{ "$flashlighttexture", "flashlight spotlight shape texture", SHADER_PARAM_TYPE_TEXTURE, "effects/flashlight001", SHADER_PARAM_NOT_EDITABLE },
	{ "$flashlighttextureframe", "Animation Frame for $flashlight", SHADER_PARAM_TYPE_INTEGER, "0", SHADER_PARAM_NOT_EDITABLE },
	{ "$color2", "color2", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", 0 },
	{ "$srgbtint", "tint value to be applied when running on new-style srgb parts", SHADER_PARAM_TYPE_COLOR, "[1 1 1]", 0 },
};

static const ShaderParamInfo_t* TryGetParam(int paramIndex)
{
	if (paramIndex < 0 || paramIndex >= Util::SafeConvert<int>(std::size(s_StandardParams)))
	{
		assert(false);
		return nullptr;
	}

	return &s_StandardParams[paramIndex];
}

CBaseShader::CBaseShader() :
	m_SoftwareVertexShader(nullptr)
{

}

int CBaseShader::GetNumParams() const
{
	return Util::SafeConvert<int>(std::size(s_StandardParams));
}

const char* CBaseShader::GetParamName(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->m_pName;

	return __FUNCTION__ "(): Out of range paramIndex";
}

const char* CBaseShader::GetParamHelp(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->m_pHelp;

	return __FUNCTION__ "(): Out of range paramIndex";
}

ShaderParamType_t CBaseShader::GetParamType(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->m_Type;

	return SHADER_PARAM_TYPE_BOOL;
}

const char* CBaseShader::GetParamDefault(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->m_pDefaultValue;

	return __FUNCTION__ "(): Out of range paramIndex";
}

int CBaseShader::GetParamFlags(int paramIndex) const
{
	if (auto param = TryGetParam(paramIndex))
		return param->m_nFlags;

	return 0;
}

void CBaseShader::InitShaderParams(IMaterialVar** params, const char* materialName)
{
	LOG_FUNC();

	assert(!s_ppParams);
	Util::ValuePusher paramsPusher(s_ppParams, params);
	return OnInitShaderParams(params, materialName);
}

void CBaseShader::InitShaderInstance(IMaterialVar** params, IShaderInit* init,
	const char* materialName, const char* textureGroupName)
{
	LOG_FUNC();

	assert(!s_ppParams);
	Util::ValuePusher paramsPusher(s_ppParams, params);

	assert(!s_pShaderInit);
	Util::ValuePusher initPusher(s_pShaderInit, init);

	assert(!s_pTextureGroupName);
	Util::ValuePusher texGroupPusher(s_pTextureGroupName, textureGroupName);

	return OnInitShaderInstance(params, init, materialName);
}

void CBaseShader::DrawElements(IMaterialVar** params, int modulationFlags, IShaderShadow* shadow,
	IShaderDynamicAPI* dynamic, VertexCompressionType_t vtxCompression, CBasePerMaterialContextData** context)
{
	LOG_FUNC();

	assert(!s_ppParams);
	Util::ValuePusher paramsPusher(s_ppParams, params);
	Util::ValuePusher shadowPusher(s_pShaderShadow, shadow);
	Util::ValuePusher dynamicPusher(s_pShaderAPI, dynamic);
	Util::ValuePusher modulationPusher(s_nModulationFlags, modulationFlags);
	Util::ValuePusher meshBuilderPusher(s_pMeshBuilder, dynamic ? dynamic->GetVertexModifyBuilder() : nullptr);

	if (IsSnapshotting())
		SetInitialShadowState();

	return OnDrawElements(params, shadow, dynamic, vtxCompression, context);
}

int CBaseShader::ComputeModulationFlags(IMaterialVar** params, IShaderDynamicAPI* dynamic)
{
	LOG_FUNC();

	// TF2Vulkan doesn't rely on snapshots the same way as default Source.
	return 0;

	Util::ValuePusher paramsPusher(s_ppParams, params);
	Util::ValuePusher dynamicPusher(s_pShaderAPI, dynamic);

	int mod = 0;
	if (UsingFlashlight(params))
		mod |= SHADER_USING_FLASHLIGHT;

	if (UsingEditor(params))
		mod |= SHADER_USING_EDITOR;

	if (IS_FLAG2_SET(MATERIAL_VAR2_USE_FIXED_FUNCTION_BAKED_LIGHTING))
	{
		assert(!"TODO: Remove all fixed-function stuff from TF2Vulkan?");
		AssertOnce(IS_FLAG2_SET(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS));
		if (IS_FLAG2_SET(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS))
		{
			mod |= SHADER_USING_FIXED_FUNCTION_BAKED_LIGHTING;
		}
	}

	if (IsSnapshotting())
	{
#if false // Not available in TF2 branch
		if (IS_FLAG2_SET(MATERIAL_VAR2_USE_GBUFFER0))
			mod |= SHADER_USING_GBUFFER0;
		if (IS_FLAG2_SET(MATERIAL_VAR2_USE_GBUFFER1))
			mod |= SHADER_USING_GBUFFER1;
#endif
	}
	else
	{
		int nFixedLightingMode = dynamic->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING);
		if (nFixedLightingMode & 1)
			mod |= SHADER_USING_GBUFFER0;
		if (nFixedLightingMode & 2)
			mod |= SHADER_USING_GBUFFER1;
	}

	return mod;
}

bool CBaseShader::NeedsPowerOfTwoFrameBufferTexture(IMaterialVar** params, bool checkSpecificToThisFrame) const
{
	return IS_FLAG2_SET(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
}

bool CBaseShader::NeedsFullFrameBufferTexture(IMaterialVar** params, bool checkSpecificToThisFrame) const
{
	return IS_FLAG2_SET(MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE);
}

bool CBaseShader::IsTranslucent(IMaterialVar** params) const
{
	return IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT);
}

void CBaseShader::Draw(bool makeActualDrawCall)
{
	LOG_FUNC();

	if (IsSnapshotting())
	{
		// Turn off transparency if we're asked to....
		if (g_pConfig->bNoTransparency &&
			((s_ppParams[FLAGS]->GetIntValue() & MATERIAL_VAR_NO_DEBUG_OVERRIDE) == 0))
		{
			s_pShaderShadow->EnableDepthWrites(true);
			s_pShaderShadow->EnableBlending(false);
		}

		GetShaderSystem()->TakeSnapshot();

#if false // Not available in TF2 branch
		// Automagically add skinning + vertex lighting
		if (!s_pInstanceDataPtr[s_nPassCount])
		{
			bool bIsSkinning = CShader_IsFlag2Set(s_ppParams, MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
			bool bIsVertexLit = CShader_IsFlag2Set(s_ppParams, MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
			if (bIsSkinning || bIsVertexLit)
			{
				PI_BeginCommandBuffer();

				// NOTE: EndCommandBuffer will insert the appropriate commands
				PI_EndCommandBuffer();
			}
		}
#endif
	}
	else
	{
		GetShaderSystem()->DrawSnapshot(makeActualDrawCall);
	}
}

void CBaseShader::SetInitialShadowState()
{
	LOG_FUNC();

	// Set the default state
	s_pShaderShadow->SetDefaultState();

	// Init the standard states...
	int flags = s_ppParams[FLAGS]->GetIntValue();
	if (flags & MATERIAL_VAR_IGNOREZ)
	{
		s_pShaderShadow->EnableDepthTest(false);
		s_pShaderShadow->EnableDepthWrites(false);
	}

	if (flags & MATERIAL_VAR_DECAL)
	{
		s_pShaderShadow->EnablePolyOffset(SHADER_POLYOFFSET_DECAL);
		s_pShaderShadow->EnableDepthWrites(false);
	}

	if (flags & MATERIAL_VAR_NOCULL)
	{
		s_pShaderShadow->EnableCulling(false);
	}

	if (flags & MATERIAL_VAR_ZNEARER)
	{
		s_pShaderShadow->DepthFunc(SHADER_DEPTHFUNC_NEARER);
	}

	if (flags & MATERIAL_VAR_WIREFRAME)
	{
		s_pShaderShadow->PolyMode(SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE);
	}

	// Set alpha to coverage
	if (flags & MATERIAL_VAR_ALLOWALPHATOCOVERAGE)
	{
		// Force the bit on and then check against alpha blend and test states in CShaderShadowDX8::ComputeAggregateShadowState()
		s_pShaderShadow->EnableAlphaToCoverage(true);
	}
}

bool CBaseShader::UsingFlashlight(IMaterialVar** params) const
{
	if (IsSnapshotting())
	{
		return IS_FLAG2_SET(MATERIAL_VAR2_USE_FLASHLIGHT);
	}
	else
	{
		if (!s_pShaderAPI)
			Error(__FUNCTION__ "(): s_pShaderAPI was nullptr");

		return s_pShaderAPI->InFlashlightMode();
	}
}

bool CBaseShader::UsingEditor(IMaterialVar** params) const
{
	if (IsSnapshotting())
	{
		return IS_FLAG2_SET(MATERIAL_VAR2_USE_EDITOR);
	}
	else
	{
		if (!s_pShaderAPI)
			Error(__FUNCTION__ "(): s_pShaderAPI was nullptr");

		return s_pShaderAPI->InEditorMode();
	}
}

void CBaseShader::BindTexture(Sampler_t sampler1, int textureVarIndex, int frameVarIndex)
{
	LOG_FUNC();

	return BindTexture(sampler1, (Sampler_t)-1, textureVarIndex, frameVarIndex);
}

void CBaseShader::BindTexture(Sampler_t sampler1, ITexture* texture, int frame)
{
	LOG_FUNC();

	return BindTexture(sampler1, (Sampler_t)-1, texture, frame);
}

void CBaseShader::BindTexture(Sampler_t sampler1, Sampler_t sampler2, ITexture* pTexture, int frame)
{
	LOG_FUNC();

	if (IsSnapshotting())
		return;

	if (sampler2 < 0)
		GetShaderSystem()->BindTexture(sampler1, pTexture, frame);
	else
		GetShaderSystem()->BindTexture(sampler1, sampler2, pTexture, frame);
}

void CBaseShader::BindTexture(Sampler_t sampler1, Sampler_t sampler2, int textureVarIndex, int frameVarIndex)
{
	LOG_FUNC();

	assert(textureVarIndex >= 0);
	assert(s_ppParams);

	if (IMaterialVar* textureVar = s_ppParams[textureVarIndex])
	{
		BindTexture(sampler1, sampler2, textureVar->GetTextureValue(),
			(frameVarIndex >= 0) ? s_ppParams[frameVarIndex]->GetIntValue() : 0);
	}
}

namespace
{
	enum class LoadType
	{
		Texture,
		Bumpmap,
		Cubemap,
	};
}

template<LoadType type>
static void LoadResource(IMaterialVar** params, int paramCount, IShaderInit* init,
	const char* texGroupName, int varIndex, int additionalFlags = 0)
{
	LOG_FUNC();

	assert(params);
	if (varIndex < 0)
		return;

	if (varIndex >= paramCount)
	{
		assert(!"Invalid parameter index");
		return;
	}

	if (auto var = params[varIndex]; var && var->IsDefined())
	{
		assert(var->GetType() == MATERIAL_VAR_TYPE_TEXTURE || var->GetType() == MATERIAL_VAR_TYPE_STRING);
		if constexpr (type == LoadType::Texture)
		{
			init->LoadTexture(var, texGroupName, additionalFlags);
		}
		else if constexpr (type == LoadType::Bumpmap)
		{
			init->LoadBumpMap(var, texGroupName);
			assert(additionalFlags == 0);
		}
		else if constexpr (type == LoadType::Cubemap)
		{
			init->LoadCubeMap(params, var, additionalFlags);
		}
	}
}

void CBaseShader::LoadTexture(int textureVarIndex, int additionalFlags)
{
	LOG_FUNC();

	return LoadResource<LoadType::Texture>(s_ppParams, GetNumParams(),
		s_pShaderInit, s_pTextureGroupName, textureVarIndex, additionalFlags);
}

void CBaseShader::LoadBumpMap(int textureVarIndex)
{
	LOG_FUNC();

	return LoadResource<LoadType::Bumpmap>(s_ppParams, GetNumParams(),
		s_pShaderInit, s_pTextureGroupName, textureVarIndex);
}

void CBaseShader::LoadCubeMap(int textureVarIndex, int additionalFlags)
{
	LOG_FUNC();

	return LoadResource<LoadType::Cubemap>(s_ppParams, GetNumParams(),
		s_pShaderInit, s_pTextureGroupName, textureVarIndex, additionalFlags);
}

bool CBaseShader::TextureIsTranslucent(int textureVar, bool isBaseTexture) const
{
	if (textureVar < 0)
		return false;

	IMaterialVar** params = s_ppParams;
	if (params[textureVar]->GetType() == MATERIAL_VAR_TYPE_TEXTURE)
	{
		if (!isBaseTexture)
		{
			return params[textureVar]->GetTextureValue()->IsTranslucent();
		}
		else
		{
			// Override translucency settings if this flag is set.
			if (IS_FLAG_SET(MATERIAL_VAR_OPAQUETEXTURE))
				return false;

			const bool bHasSelfIllum = ((CurrentMaterialVarFlags() & MATERIAL_VAR_SELFILLUM) != 0);
			const bool bHasSelfIllumMask = false;// ((CurrentMaterialVarFlags2() & MATERIAL_VAR2_SELFILLUMMASK) != 0);
			const bool bHasBaseAlphaEnvmapMask = ((CurrentMaterialVarFlags() & MATERIAL_VAR_BASEALPHAENVMAPMASK) != 0);
			const bool bUsingBaseTextureAlphaForSelfIllum = bHasSelfIllum && !bHasSelfIllumMask;
			// Check if we are using base texture alpha for something other than translucency.
			if (!bUsingBaseTextureAlphaForSelfIllum && !bHasBaseAlphaEnvmapMask)
			{
				// We aren't using base alpha for anything other than trancluceny.

				// check if the material is marked as translucent or alpha test.
				if ((CurrentMaterialVarFlags() & MATERIAL_VAR_TRANSLUCENT) ||
					(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST))
				{
					// Make sure the texture has an alpha channel.
					return params[textureVar]->GetTextureValue()->IsTranslucent();
				}
			}
		}
	}

	return false;
}

void CBaseShader::SetBlendingShadowState(BlendType_t nMode)
{
	switch (nMode)
	{
	case BT_NONE:      return DisableAlphaBlending();
	case BT_BLEND:     return EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
	case BT_ADD:       return EnableAlphaBlending(SHADER_BLEND_ONE, SHADER_BLEND_ONE);
	case BT_BLENDADD:  return EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE);
	}

	assert(!"Invalid BlendType_t");
}

bool CBaseShader::IsAlphaModulating() const
{
	return (s_nModulationFlags & SHADER_USING_ALPHA_MODULATION) != 0;
}

void CBaseShader::EnableAlphaBlending(ShaderBlendFactor_t src, ShaderBlendFactor_t dst)
{
	Assert(IsSnapshotting());
	s_pShaderShadow->EnableBlending(true);
	s_pShaderShadow->BlendFunc(src, dst);
	s_pShaderShadow->EnableDepthWrites(false);
}

void CBaseShader::DisableAlphaBlending()
{
	Assert(IsSnapshotting());
	s_pShaderShadow->EnableBlending(false);
}
