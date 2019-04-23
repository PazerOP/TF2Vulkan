#include "TF2Vulkan/Material.h"
#include "Util/Placeholders.h"

#include <materialsystem/imaterialvar.h>
#include <tier1/KeyValues.h>

using namespace TF2Vulkan;

static CUtlSymbol GetShaderName(const KeyValues& kv)
{
	auto name = kv.GetName();
	kv.GetNameSymbol();
	return CUtlSymbol(name);
}

Material::Material(const KeyValues& kv, const CUtlSymbol& name, const CUtlSymbol& textureGroupName) :
	m_Name(name),
	m_TextureGroupName(textureGroupName),
	m_ShaderName(::GetShaderName(kv))
{
	// Workaround for broken const correctness
	KeyValues::AutoDelete kv2(kv.MakeCopy());

	for (auto it = kv2->GetFirstValue(); it; it = it->GetNextValue())
	{
		auto varName = it->GetName();
		auto varValue = it->GetString();
		m_Vars[varName] = std::make_unique<MaterialVar>(this, varName, varValue);
	}
}

PreviewImageRetVal_t Material::GetPreviewImageProperties(int* width, int* height, ImageFormat* format, bool* isTranslucent) const
{
	NOT_IMPLEMENTED_FUNC();
	return PreviewImageRetVal_t();
}

PreviewImageRetVal_t Material::GetPreviewImage(unsigned char* data, int width, int height, ImageFormat format) const
{
	NOT_IMPLEMENTED_FUNC();
	return PreviewImageRetVal_t();
}

int Material::GetMappingWidth() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int Material::GetMappingHeight() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int Material::GetNumAnimationFrames() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool Material::InMaterialPage() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void Material::GetMaterialOffset(float* offset) const
{
	NOT_IMPLEMENTED_FUNC();
}

void Material::GetMaterialScale(float* scale) const
{
	NOT_IMPLEMENTED_FUNC();
}

IMaterial* Material::GetMaterialPage()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

IMaterialVar* Material::FindVar(const char* varName, bool* found, bool complain)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

IMaterialVar* Material::FindVarFast(const char* pVarName, unsigned int* token)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

void Material::IncrementReferenceCount()
{
	++m_RefCount;
}

void Material::DecrementReferenceCount()
{
#ifdef _DEBUG
	auto oldVal = m_RefCount--;
	assert(oldVal > 0);
#else
	--m_RefCount;
#endif
}

int Material::GetEnumerationID() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void Material::GetLowResColorSample(float s, float t, float* color) const
{
	NOT_IMPLEMENTED_FUNC();
}

void Material::RecomputeStateSnapshots()
{
	NOT_IMPLEMENTED_FUNC();
}

bool Material::IsTranslucent() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::IsAlphaTested() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::IsVertexLit() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

VertexFormat_t Material::GetVertexFormat() const
{
	NOT_IMPLEMENTED_FUNC();
	return VertexFormat_t();
}

bool Material::HasProxy() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::UsesEnvCubemap() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::NeedsTangentSpace() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::NeedsPowerOfTwoFrameBufferTexture(bool checkSpecificToThisFrame) const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::NeedsFullFrameBufferTexture(bool checkSpecificToThisFrame) const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::NeedsSoftwareSkinning() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void Material::AlphaModulate(float alpha)
{
	NOT_IMPLEMENTED_FUNC();
}

void Material::ColorModulate(float r, float g, float b)
{
	NOT_IMPLEMENTED_FUNC();
}

float Material::GetAlphaModulation() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0.0f;
}

void Material::GetColorModulation(float* r, float* g, float* b) const
{
	NOT_IMPLEMENTED_FUNC();
}

void Material::SetMaterialVarFlag(MaterialVarFlags_t flag, bool on)
{
	NOT_IMPLEMENTED_FUNC();
}

bool Material::GetMaterialVarFlag(MaterialVarFlags_t flag) const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void Material::GetReflectivity(Vector& reflect) const
{
	NOT_IMPLEMENTED_FUNC();
}

bool Material::GetPropertyFlag(MaterialPropertyTypes_t type) const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::IsTwoSided() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void Material::SetShader(const char* shaderName)
{
	NOT_IMPLEMENTED_FUNC();
}

void Material::SetShaderAndParams(KeyValues* keyValues)
{
	NOT_IMPLEMENTED_FUNC();
}

int Material::GetNumPasses() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

int Material::GetTextureMemoryBytes() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

void Material::Refresh()
{
	// TODO
	//NOT_IMPLEMENTED_FUNC();
}

void Material::RefreshPreservingMaterialVars()
{
	// TODO
	//NOT_IMPLEMENTED_FUNC();
}

bool Material::NeedsLightmapBlendAlpha() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::NeedsSoftwareLighting() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

int Material::ShaderParamCount() const
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

IMaterialVar** Material::GetShaderParams()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool Material::IsErrorMaterial() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void Material::SetUseFixedFunctionBakedLighting(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

MorphFormat_t Material::GetMorphFormat() const
{
	NOT_IMPLEMENTED_FUNC();
	return MorphFormat_t();
}

void Material::DeleteIfUnreferenced()
{
	NOT_IMPLEMENTED_FUNC();
}

bool Material::IsSpriteCard() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

void Material::CallBindProxy(void* proxyData)
{
	NOT_IMPLEMENTED_FUNC();
}

IMaterial* Material::CheckProxyReplacement(void* proxyData)
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

bool Material::WasReloadedFromWhitelist() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool Material::IsPrecached() const
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}
