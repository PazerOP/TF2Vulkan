#include "stdshader_dx9_tf2vulkan/ShaderBlobs.h"

#include <cassert>
#include <cstdint>

using namespace TF2Vulkan;

namespace
{
#include "Generated/vertexlit_and_unlit_generic.vert.h"
}

bool TF2Vulkan::GetShaderBlob(ShaderBlob type, const void*& data, size_t& size)
{
	switch (type)
	{
	default:
		assert(!"Unknown blob type");
		return false;

	case ShaderBlob::VertexLitAndUnlitGeneric_VS:
	{
		data = vertexlit_and_unlit_generic_vert_spirv;
		size = vertexlit_and_unlit_generic_vert_spirv_len;
		return true;
	}
	}
}
