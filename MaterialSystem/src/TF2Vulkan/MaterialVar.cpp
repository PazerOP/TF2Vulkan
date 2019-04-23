#include "TF2Vulkan/Material.h"
#include "TF2Vulkan/MaterialVar.h"
#include "TF2Vulkan/MaterialSystem.h"
#include <TF2Vulkan/Util/Placeholders.h>
#include <TF2Vulkan/Util/std_algorithm.h>
#include <TF2Vulkan/Util/std_charconv.h>

#include <materialsystem/imaterial.h>
#include <materialsystem/imaterialsystem.h>
#include <materialsystem/itexture.h>

using namespace TF2Vulkan;

static const VMatrix s_DummyMatrix = SetupMatrixIdentity();

struct MaterialVar::FourCCData
{
	FourCC m_Type = FOURCC_UNKNOWN;
	void* m_Data = nullptr;
};

struct MaterialVar::MatrixData
{
	VMatrix m_Matrix;
};

MaterialVar::MaterialVar(Material* parent, const char* name, const char* value) :
	m_Parent(parent)
{
	m_Type = MATERIAL_VAR_TYPE_UNDEFINED;
	m_Name = name;
	m_pStringVal = nullptr;
	assert(m_Parent);

	// Explicit, so this isn't a virtual function call
	MaterialVar::SetValueAutodetectType(value);
}

const char* MaterialVar::GetName() const
{
	return m_Name.String();
}

MaterialVarSym_t MaterialVar::GetNameAsSymbol() const
{
	return m_Name;
}

ITexture* MaterialVar::GetTextureValue()
{
	if (!IsTexture())
	{
		assert(false);
		return nullptr;
	}

	return GetMaterialSystem()->FindTexture(GetStringValue(), nullptr);
}

void MaterialVar::SetTextureValue(ITexture* texture)
{
	if (texture)
	{
		ChangeTypeTo(MATERIAL_VAR_TYPE_TEXTURE);
		SetStringImpl(texture->GetName());
		m_ExtendedValue.m_Texture = texture;
	}
	else
	{
		SetUndefined();
	}
}

IMaterial* MaterialVar::GetMaterialValue()
{
	if (GetType() != MATERIAL_VAR_TYPE_MATERIAL)
	{
		assert(false);
		return nullptr;
	}

	return GetMaterialSystem()->FindMaterial(GetStringValue(), nullptr);
}

void MaterialVar::SetMaterialValue(IMaterial* material)
{
	if (material)
	{
		ChangeTypeTo(MATERIAL_VAR_TYPE_MATERIAL);
		SetStringImpl(material->GetName());
		m_ExtendedValue.m_Material = material;
	}
	else
	{
		SetUndefined();
	}
}

bool MaterialVar::IsDefined() const
{
	return GetType() != MATERIAL_VAR_TYPE_UNDEFINED;
}

void MaterialVar::SetUndefined()
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_UNDEFINED);
}

bool MaterialVar::HasStringRep(MaterialVarType_t type)
{
	switch (type)
	{
		// All of these have valid string representations
	case MATERIAL_VAR_TYPE_FLOAT:
	case MATERIAL_VAR_TYPE_STRING:
	case MATERIAL_VAR_TYPE_VECTOR:
	case MATERIAL_VAR_TYPE_TEXTURE:
	case MATERIAL_VAR_TYPE_INT:
	case MATERIAL_VAR_TYPE_MATERIAL:
		return true;

		// These do not have valid string representations
	case MATERIAL_VAR_TYPE_FOURCC:
	case MATERIAL_VAR_TYPE_MATRIX:
	case MATERIAL_VAR_TYPE_UNDEFINED:
		return false;
	}

	assert(!"Unknown type");
	return false;
}

void MaterialVar::ChangeTypeTo(MaterialVarType_t newType)
{
	const auto oldType = GetType();
	if (newType != oldType)
	{
		switch (oldType)
		{
		default:
			assert(!"Unknown type");
		case MATERIAL_VAR_TYPE_STRING:
		case MATERIAL_VAR_TYPE_FLOAT:
		case MATERIAL_VAR_TYPE_VECTOR:
		case MATERIAL_VAR_TYPE_INT:
		case MATERIAL_VAR_TYPE_UNDEFINED:
			break; // Do nothing

		case MATERIAL_VAR_TYPE_MATRIX:
			delete m_ExtendedValue.m_Matrix;
			m_ExtendedValue.m_Matrix = nullptr;
			break;
		case MATERIAL_VAR_TYPE_FOURCC:
			delete m_ExtendedValue.m_FourCC;
			m_ExtendedValue.m_FourCC = nullptr;
			break;
		case MATERIAL_VAR_TYPE_MATERIAL:
			m_ExtendedValue.m_Material = nullptr;
			break;
		case MATERIAL_VAR_TYPE_TEXTURE:
			m_ExtendedValue.m_Texture = nullptr;
			break;
		}

		ClearStringValue();
	}
}

void MaterialVar::ClearStringValue()
{
	if (m_pStringVal)
	{
		free(m_pStringVal);
		m_pStringVal = nullptr;
	}
}

void MaterialVar::SetStringImpl(const char* str)
{
	ClearStringValue();
	m_pStringVal = strdup(str);
}

const char* MaterialVar::GetStringValue() const
{
	return m_pStringVal ? m_pStringVal : "";
}

void MaterialVar::SetStringValue(const char* val)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_STRING);
	SetStringImpl(val);
}

void MaterialVar::GetFourCCValue(FourCC* type, const void** data) const
{
	if (GetType() == MATERIAL_VAR_TYPE_FOURCC)
	{
		*type = m_ExtendedValue.m_FourCC->m_Type;
		*data = m_ExtendedValue.m_FourCC->m_Data;
	}
	else
	{
		*type = FOURCC_UNKNOWN;
		*data = nullptr;
		assert(!"Attempted to access non-fourcc data as fourcc data");
	}
}

void MaterialVar::SetFourCCValue(FourCC type, void* data)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_FOURCC);

	m_ExtendedValue.m_FourCC->m_Type = type;
	m_ExtendedValue.m_FourCC->m_Data = data;
}

void MaterialVar::SetFloatValue(float val)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_FLOAT);
	m_VecVal.x = val;
	m_intVal = val;
}

void MaterialVar::SetIntValue(int val)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_INT);
	m_VecVal.x = val;
	m_intVal = val;
}

void MaterialVar::GetLinearVecValue(float* val, int components) const
{
	NOT_IMPLEMENTED_FUNC();
}

void MaterialVar::SetVecValue(const float* val, int components)
{
	switch (components)
	{
	case 1: return SetFloatValue(val[0]);
	case 2: return SetVecValue(val[0], val[1]);
	case 3: return SetVecValue(val[0], val[1], val[2]);

	default:
		assert(!"Invalid component count");

	case 4: return SetVecValue(val[0], val[1], val[2], val[3]);
	}
}

void MaterialVar::SetVecValue(float x, float y)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_VECTOR);

	m_intVal = x;

	m_VecVal.x = x;
	m_VecVal.y = y;
}

void MaterialVar::SetVecValue(float x, float y, float z)
{
	SetVecValue(x, y);
	m_VecVal.z = z;
}

void MaterialVar::SetVecValue(float x, float y, float z, float w)
{
	SetVecValue(x, y, z);
	m_VecVal.w = w;
}

void MaterialVar::SetVecComponentValue(float val, int component)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_VECTOR);

	if (component >= VEC_COMPONENT_COUNT)
	{
		assert(false);
		component = VEC_COMPONENT_COUNT - 1;
	}
	else if (component < 0)
	{
		assert(false);
		component = 0;
	}

	if (component == 0)
		m_intVal = val;

	m_VecVal[component] = val;
}

const VMatrix& MaterialVar::GetMatrixValue() const
{
	if (GetType() == MATERIAL_VAR_TYPE_MATRIX)
	{
		return m_ExtendedValue.m_Matrix->m_Matrix;
	}
	else
	{
		assert(!"Type mismatch");
		return s_DummyMatrix;
	}
}

void MaterialVar::SetMatrixValue(const VMatrix& matrix)
{
	ChangeTypeTo(MATERIAL_VAR_TYPE_MATRIX);
	m_ExtendedValue.m_Matrix->m_Matrix = matrix;
}

bool MaterialVar::MatrixIsIdentity() const
{
	if (GetType() == MATERIAL_VAR_TYPE_MATRIX)
	{
		return m_ExtendedValue.m_Matrix->m_Matrix == s_DummyMatrix;
	}
	else
	{
		assert(!"Type mismatch");
		return false;
	}
}

IMaterial* MaterialVar::GetOwningMaterial()
{
	return m_Parent;
}

void MaterialVar::CopyFrom(const IMaterialVar* other)
{
	switch (other->GetType())
	{
	case MATERIAL_VAR_TYPE_FLOAT:
		return SetFloatValue(other->GetFloatValue());
	case MATERIAL_VAR_TYPE_STRING:
		return SetStringValue(other->GetStringValue());
	case MATERIAL_VAR_TYPE_VECTOR:
		return SetVecValue(other->GetVecValue(), other->VectorSize());
	case MATERIAL_VAR_TYPE_TEXTURE:
		return SetTextureValue(const_cast<IMaterialVar*>(other)->GetTextureValue());
	case MATERIAL_VAR_TYPE_INT:
		return SetIntValue(other->GetIntValue());
	case MATERIAL_VAR_TYPE_FOURCC:
	{
		FourCC type;
		void* data;
		const_cast<IMaterialVar*>(other)->GetFourCCValue(&type, &data);
		return SetFourCCValue(type, data);
	}

	case MATERIAL_VAR_TYPE_MATRIX:
		return SetMatrixValue(const_cast<IMaterialVar*>(other)->GetMatrixValue());
	case MATERIAL_VAR_TYPE_MATERIAL:
		return SetMaterialValue(const_cast<IMaterialVar*>(other)->GetMaterialValue());

	default:
		assert(!"Unknown type");
	case MATERIAL_VAR_TYPE_UNDEFINED:
		return SetUndefined();
	}
}

static void EatWhitespace(std::string_view& input)
{
	for (auto it = input.begin(); it != input.end(); ++it)
	{
		if (!isspace(*it))
		{
			input = input.substr(it - input.begin());
			return;
		}
	}
}

void MaterialVar::SetValueAutodetectType(const char* rawVal)
{
	std::string_view val(rawVal);
	EatWhitespace(val);

	float tempFloat;
	int tempInt;

	bool wasString = false;

	if (val.empty())
		SetUndefined();
	else if (val[0] == '[')
	{
		val = val.substr(1);

		int componentCount = 0;
		float components[VEC_COMPONENT_COUNT];
		for (; componentCount < VEC_COMPONENT_COUNT; componentCount++)
		{
			EatWhitespace(val);

			auto result = Util::charconv::from_chars(val, components[componentCount]);
			if (!result)
				break;

			val = result.m_Remaining;
		}

		if (val.empty() || val.at(0) != ']')
			Warning("[TF2Vulkan] Missing closing ']' when parsing \"%s\" for material var \"%s\" in material \"%s\"\n",
				rawVal, GetName(), GetOwningMaterial()->GetName());

		if (componentCount > 0)
			SetVecValue(components, componentCount);
		else
			SetUndefined();
	}
	else if (Util::algorithm::contains(val, '.') && sscanf_s(rawVal, "%f", &tempFloat) == 1)
	{
		SetFloatValue(tempFloat);
	}
	else if (sscanf_s(rawVal, "%i", &tempInt) == 1)
	{
		SetIntValue(tempInt);
	}
	else
	{
		SetStringValue(rawVal);
		wasString = true;
	}

	if (!wasString)
		SetStringImpl(rawVal);
}

int MaterialVar::GetIntValueInternal() const
{
	return m_intVal;
}

float MaterialVar::GetFloatValueInternal() const
{
	return m_VecVal.x;
}

const float* MaterialVar::GetVecValueInternal() const
{
	return m_VecVal.Base();
}

void MaterialVar::GetVecValueInternal(float* val, int components) const
{
	if (components > VEC_COMPONENT_COUNT)
	{
		assert(false);
		components = VEC_COMPONENT_COUNT;
	}

	assert(components > 0);

	for (int i = 0; i < components; i++)
		val[i] = m_VecVal[i];
}

int MaterialVar::VectorSizeInternal() const
{
	static_assert(sizeof(m_VecVal) == (VEC_COMPONENT_COUNT * sizeof(float)));
	return VEC_COMPONENT_COUNT;
}
