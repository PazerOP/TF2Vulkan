#pragma once
#include <TF2Vulkan/Util/std_utility.h>

#include <materialsystem/imaterial.h>

namespace TF2Vulkan
{
	// Const-correctness fixes
	class IMaterial2 : public IMaterial
	{
	public:
		virtual int GetMappingWidth() const = 0;
		int GetMappingWidth() override final { return Util::as_const(this)->GetMappingWidth(); }

		virtual int GetMappingHeight() const = 0;
		int GetMappingHeight() override final { return Util::as_const(this)->GetMappingHeight(); }

		virtual int GetNumAnimationFrames() const = 0;
		int GetNumAnimationFrames() override final { return Util::as_const(this)->GetNumAnimationFrames(); }

		virtual bool InMaterialPage() const = 0;
		bool InMaterialPage() override final { return Util::as_const(this)->InMaterialPage(); }

		virtual void GetMaterialOffset(float* offset) const = 0;
		void GetMaterialOffset(float* offset) override final { Util::as_const(this)->GetMaterialOffset(offset); }

		virtual void GetMaterialScale(float* scale) const = 0;
		void GetMaterialScale(float* scale) override final { Util::as_const(this)->GetMaterialScale(scale); }

		virtual bool IsTranslucent() const = 0;
		bool IsTranslucent() override final { return Util::as_const(this)->IsTranslucent(); }

		virtual bool IsAlphaTested() const = 0;
		bool IsAlphaTested() override final { return Util::as_const(this)->IsAlphaTested(); }

		virtual bool IsVertexLit() const = 0;
		bool IsVertexLit() override final { return Util::as_const(this)->IsVertexLit(); }

		virtual bool UsesEnvCubemap() const = 0;
		bool UsesEnvCubemap() override final { return Util::as_const(this)->UsesEnvCubemap(); }

		virtual bool NeedsTangentSpace() const = 0;
		bool NeedsTangentSpace() override final { return Util::as_const(this)->NeedsTangentSpace(); }

		virtual bool NeedsPowerOfTwoFrameBufferTexture(bool checkSpecificToThisFrame) const = 0;
		bool NeedsPowerOfTwoFrameBufferTexture(bool checkSpecificToThisFrame) override final
		{
			return Util::as_const(this)->NeedsPowerOfTwoFrameBufferTexture(checkSpecificToThisFrame);
		}

		virtual bool NeedsFullFrameBufferTexture(bool checkSpecificToThisFrame) const = 0;
		bool NeedsFullFrameBufferTexture(bool checkSpecificToThisFrame) override final
		{
			return Util::as_const(this)->NeedsFullFrameBufferTexture(checkSpecificToThisFrame);
		}

		virtual bool NeedsSoftwareSkinning() const = 0;
		bool NeedsSoftwareSkinning() override final { return Util::as_const(this)->NeedsSoftwareSkinning(); }

		virtual void GetReflectivity(Vector& reflect) const = 0;
		void GetReflectivity(Vector& reflect) override final { Util::as_const(this)->GetReflectivity(reflect); }

		virtual bool GetPropertyFlag(MaterialPropertyTypes_t type) const = 0;
		bool GetPropertyFlag(MaterialPropertyTypes_t type) override final { return Util::as_const(this)->GetPropertyFlag(type); }

#if true
		virtual bool IsTwoSided() const = 0;
		bool IsTwoSided() override final { return Util::as_const(this)->IsTwoSided(); }

		virtual int GetNumPasses() const = 0;
		int GetNumPasses() override final { return Util::as_const(this)->GetNumPasses(); }

		virtual int GetTextureMemoryBytes() const = 0;
		int GetTextureMemoryBytes() override final { return Util::as_const(this)->GetTextureMemoryBytes(); }
#endif

		virtual bool NeedsLightmapBlendAlpha() const = 0;
		bool NeedsLightmapBlendAlpha() override final { return Util::as_const(this)->NeedsLightmapBlendAlpha(); }

		virtual bool NeedsSoftwareLighting() const = 0;
		bool NeedsSoftwareLighting() override final { return Util::as_const(this)->NeedsSoftwareLighting(); }

		virtual float GetAlphaModulation() const = 0;
		float GetAlphaModulation() override final { return Util::as_const(this)->GetAlphaModulation(); }

		virtual void GetColorModulation(float* r, float* g, float* b) const = 0;
		void GetColorModulation(float* r, float* g, float* b) override final
		{
			return Util::as_const(this)->GetColorModulation(r, g, b);
		}

		virtual bool IsSpriteCard() const = 0;
		bool IsSpriteCard() override final { return Util::as_const(this)->IsSpriteCard(); }

		virtual bool WasReloadedFromWhitelist() const = 0;
		bool WasReloadedFromWhitelist() override final { return Util::as_const(this)->WasReloadedFromWhitelist(); }
	};
}
