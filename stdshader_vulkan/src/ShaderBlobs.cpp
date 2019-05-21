#include "stdshader_vulkan/ShaderBlobs.h"
#include <TF2Vulkan/Util/interface.h>

#include <cassert>
#include <cstdint>
#include <iterator>

using namespace TF2Vulkan;

namespace
{
#include "shaders/Generated/bik.vert.h"
#include "shaders/Generated/bik.frag.h"
#include "shaders/Generated/bufferclearobeystencil.vert.h"
#include "shaders/Generated/bufferclearobeystencil.frag.h"
#include "shaders/Generated/fillrate.vert.h"
#include "shaders/Generated/fillrate.frag.h"
#include "shaders/Generated/xlitgeneric.vert.h"
#include "shaders/Generated/xlitgeneric.frag.h"

#define SHADER_CASE(type, varName) \
	case ShaderBlobType:: ## type: \
	{ \
		data = (varName); \
		size = std::size((varName)); \
		return true; \
	}

	class ShaderBlobs final : public IShaderBlobs
	{
	public:
		bool GetShaderBlob(ShaderBlobType type, const void*& data, size_t& size) const override
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
	};
}

EXPOSE_SINGLE_INTERFACE(ShaderBlobs, IShaderBlobs, SHADER_BLOBS_INTERFACE_VERSION);
