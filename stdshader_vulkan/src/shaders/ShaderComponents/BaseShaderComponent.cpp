#include "BaseShaderComponent.h"

#include "TF2Vulkan/Util/std_charconv.h"

void TF2Vulkan::Shaders::detail::LastDitchInitParams(IMaterialVar** params, const ShaderParamNext* base, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		auto& p = base[i];
		assert(p.GetIndex() >= 0);
		if (params[p]->IsDefined())
			continue;

		switch (p.GetType())
		{
		default:
			assert(!"Unknown shader param type");

		case SHADER_PARAM_TYPE_COLOR:
		case SHADER_PARAM_TYPE_VEC2:
		case SHADER_PARAM_TYPE_VEC3:
		case SHADER_PARAM_TYPE_VEC4:
		case SHADER_PARAM_TYPE_MATRIX:
		case SHADER_PARAM_TYPE_MATRIX4X2:
			params[p]->SetValueAutodetectType(p.GetDefault());
			break;

		case SHADER_PARAM_TYPE_FLOAT:
			params[p]->SetFloatValue(Util::charconv::from_chars_v<float>(p.GetDefault()).value());
			break;
		case SHADER_PARAM_TYPE_INTEGER:
			params[p]->SetIntValue(Util::charconv::from_chars_v<int>(p.GetDefault()).value());
			break;
		case SHADER_PARAM_TYPE_BOOL:
		{
			auto val = Util::charconv::from_chars_v<int>(p.GetDefault()).value();
			assert(val == 0 || val == 1);
			params[p]->SetIntValue(val ? 0 : 1);
			break;
		}

		case SHADER_PARAM_TYPE_STRING:
			params[p]->SetStringValue(p.GetDefault());
			break;

		case SHADER_PARAM_TYPE_TEXTURE:
			break; // Do nothing for textures (we don't want textures set if they're not begin used
		}
	}
}
