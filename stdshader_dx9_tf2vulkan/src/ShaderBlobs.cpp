#include "stdshader_dx9_tf2vulkan/ShaderBlobs.h"

#include <cassert>
#include <cstdint>
#include <iterator>

using namespace TF2Vulkan;

namespace
{
#include "Generated/vertexlit_and_unlit_generic.vert.h"
#include "Generated/vertexlit_and_unlit_generic.frag.h"
}

#define SHADER_CASE(type, varName) \
case ShaderBlob:: type: \
{ \
	data = (varName); \
	size = std::size((varName)); \
	return true; \
}

bool TF2Vulkan::GetShaderBlob(ShaderBlob type, const void*& data, size_t& size)
{
	switch (type)
	{
	default:
		assert(!"Unknown blob type");
		return false;

		SHADER_CASE(VertexLitAndUnlitGeneric_VS, vertexlit_and_unlit_generic_vert_spirv);
		SHADER_CASE(VertexLitAndUnlitGeneric_PS, vertexlit_and_unlit_generic_frag_spirv);
	}
}
