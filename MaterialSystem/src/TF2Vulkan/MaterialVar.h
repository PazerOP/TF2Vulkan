#pragma once

#include "IMaterialVar2.h"

#include <string>
#include <variant>

namespace TF2Vulkan
{
	class Material;

	class MaterialVar final : public IMaterialVar2
	{
		static constexpr int VEC_COMPONENT_COUNT = 4;

	public:
		MaterialVar(Material* parent, const char* name, const char* value);

		const char* GetName() const;
		MaterialVarSym_t GetNameAsSymbol() const override;

		ITexture* GetTextureValue() override;
		void SetTextureValue(ITexture* texture) override;

		IMaterial* GetMaterialValue() override;
		void SetMaterialValue(IMaterial* material) override;

		bool IsDefined() const override;
		void SetUndefined() override;

		const char* GetStringValue() const override;
		void SetStringValue(const char* val) override;

		using IMaterialVar2::GetFourCCValue;
		void GetFourCCValue(FourCC* type, const void** data) const override;
		void SetFourCCValue(FourCC type, void* data) override;

		void SetFloatValue(float val) override;
		void SetIntValue(int val) override;

		void GetLinearVecValue(float* val, int components) const override;
		void SetVecValue(const float* val, int components) override;
		void SetVecValue(float x, float y) override;
		void SetVecValue(float x, float y, float z) override;
		void SetVecValue(float x, float y, float z, float w) override;
		void SetVecComponentValue(float val, int component) override;

		const VMatrix& GetMatrixValue() const override;
		void SetMatrixValue(const VMatrix& matrix) override;
		bool MatrixIsIdentity() const override;

		IMaterial* GetOwningMaterial() override;

		void CopyFrom(const IMaterialVar* other) override;

		void SetValueAutodetectType(const char* val) override;

	protected:
		int GetIntValueInternal() const override;
		float GetFloatValueInternal() const override;
		const float* GetVecValueInternal() const override;
		void GetVecValueInternal(float* val, int components) const override;
		int VectorSizeInternal() const override;

	private:
		static bool HasStringRep(MaterialVarType_t type);

		Material* m_Parent = nullptr;

		void ChangeTypeTo(MaterialVarType_t type);
		void SetStringImpl(const char* str);
		void ClearStringValue();

		struct FourCCData;
		struct MatrixData;

		union
		{
			IMaterial* m_Material = nullptr;
			ITexture* m_Texture;
			FourCCData* m_FourCC;
			MatrixData* m_Matrix;

		} m_ExtendedValue;

		static_assert(sizeof(m_ExtendedValue) == sizeof(m_ExtendedValue.m_Material));
	};
}
