#pragma once

#include "ISpecConstLayout.h"
#include <shaderapi/ishaderapi.h>

namespace TF2Vulkan
{
	class IUniformBufferManager;
	class IShaderAPINext
	{
	public:
		virtual IUniformBufferManager* FindOrCreateUBM(size_t size) = 0;
	};
}
