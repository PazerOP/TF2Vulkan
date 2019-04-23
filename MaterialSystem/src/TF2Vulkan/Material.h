#pragma once

#include "TF2Vulkan/IMaterial2.h"
#include "TF2Vulkan/MaterialVar.h"

#include <TF2Vulkan/Util/utlsymbol.h>

#include <tier1/utlsymbol.h>

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

namespace TF2Vulkan
{
	class Material final : public IMaterial2
	{
	public:
		Material(const KeyValues& kv, const CUtlSymbol& name, const CUtlSymbol& textureGroupName);

		const char* GetName() const override { return m_Name.String(); }
		const char* GetTextureGroupName() const override { return m_TextureGroupName.String(); }

		PreviewImageRetVal_t GetPreviewImageProperties(int* width, int* height,
			ImageFormat* format, bool* isTranslucent) const override;

		PreviewImageRetVal_t GetPreviewImage(unsigned char* data,
			int width, int height, ImageFormat format) const override;

		int GetMappingWidth() const override;
		int GetMappingHeight() const override;

		int GetNumAnimationFrames() const override;

		bool InMaterialPage() const override;
		void GetMaterialOffset(float* offset) const override;
		void GetMaterialScale(float* scale) const override;
		IMaterial* GetMaterialPage() override;

		IMaterialVar* FindVar(const char* varName, bool* found, bool complain) override;
		IMaterialVar* FindVarFast(const char* pVarName, unsigned int* token) override;

		void IncrementReferenceCount() override;
		void DecrementReferenceCount() override;

		int GetEnumerationID() const override;

		void GetLowResColorSample(float s, float t, float* color) const override;

		void RecomputeStateSnapshots() override;

		bool IsTranslucent() const override;
		bool IsAlphaTested() const override;
		bool IsVertexLit() const override;
		VertexFormat_t GetVertexFormat() const override;
		bool HasProxy() const override;
		bool UsesEnvCubemap() const override;
		bool NeedsTangentSpace() const override;

		bool NeedsPowerOfTwoFrameBufferTexture(bool checkSpecificToThisFrame) const override;
		bool NeedsFullFrameBufferTexture(bool checkSpecificToThisFrame) const override;

		bool NeedsSoftwareSkinning() const override;

		void AlphaModulate(float alpha) override;
		void ColorModulate(float r, float g, float b) override;
		float GetAlphaModulation() const override;
		void GetColorModulation(float* r, float* g, float* b) const override;

		void SetMaterialVarFlag(MaterialVarFlags_t flag, bool on) override;
		bool GetMaterialVarFlag(MaterialVarFlags_t flag) const override;

		void GetReflectivity(Vector& reflect) const override;

		bool GetPropertyFlag(MaterialPropertyTypes_t type) const override;

		bool IsTwoSided() const override;

		void SetShader(const char* shaderName) override;
		void SetShaderAndParams(KeyValues* keyValues) override;
		const char* GetShaderName() const { return m_ShaderName.String(); }

		int GetNumPasses() const override;
		int GetTextureMemoryBytes() const override;

		void Refresh() override;
		void RefreshPreservingMaterialVars() override;

		bool NeedsLightmapBlendAlpha() const override;
		bool NeedsSoftwareLighting() const override;

		int ShaderParamCount() const override;
		IMaterialVar** GetShaderParams() override;

		bool IsErrorMaterial() const override;

		void SetUseFixedFunctionBakedLighting(bool enable) override;

		MorphFormat_t GetMorphFormat() const override;

		void DeleteIfUnreferenced() override;

		bool IsSpriteCard() const override;

		void CallBindProxy(void* proxyData) override;
		IMaterial* CheckProxyReplacement(void* proxyData) override;

		bool WasReloadedFromWhitelist() const override;
		bool IsPrecached() const override;

	private:
		std::atomic<uint_fast32_t> m_RefCount;

		CUtlSymbolDbg m_Name;
		CUtlSymbolDbg m_TextureGroupName;
		CUtlSymbolDbg m_ShaderName;

		std::unordered_map<CUtlSymbol, std::unique_ptr<MaterialVar>> m_Vars;
	};
}
