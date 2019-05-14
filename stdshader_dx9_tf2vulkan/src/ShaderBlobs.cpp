#include "stdshader_dx9_tf2vulkan/ShaderBlobs.h"

#include <cassert>
#include <cstdint>
#include <iterator>

using namespace TF2Vulkan;

namespace
{
#include "Generated/bik.vert.h"
#include "Generated/bik.frag.h"
#include "Generated/bufferclearobeystencil.vert.h"
#include "Generated/bufferclearobeystencil.frag.h"
#include "Generated/fillrate.vert.h"
#include "Generated/fillrate.frag.h"
#include "Generated/xlitgeneric.vert.h"
#include "Generated/xlitgeneric.frag.h"
}

#define SHADER_CASE(type, varName) \
case ShaderBlob:: ## type: \
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

		SHADER_CASE(Bik_VS, bik_vert_spirv);
		SHADER_CASE(Bik_PS, bik_frag_spirv);
		SHADER_CASE(BufferClearObeyStencil_VS, bufferclearobeystencil_vert_spirv);
		SHADER_CASE(BufferClearObeyStencil_PS, bufferclearobeystencil_frag_spirv);
		SHADER_CASE(Fillrate_VS, fillrate_vert_spirv);
		SHADER_CASE(Fillrate_PS, fillrate_frag_spirv);
		SHADER_CASE(XLitGeneric_VS, xlitgeneric_vert_spirv);
		SHADER_CASE(XLitGeneric_PS, xlitgeneric_frag_spirv);
	}
}
