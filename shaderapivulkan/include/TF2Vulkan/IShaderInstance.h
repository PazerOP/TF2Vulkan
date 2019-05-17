#pragma once

#include <TF2Vulkan/Util/std_utility.h>

namespace TF2Vulkan
{
	class IShaderGroup;
	class IShaderInstance
	{
	public:
		IShaderGroup& GetGroup() { return const_cast<IShaderGroup&>(Util::as_const(this)->GetGroup()); }
		virtual const IShaderGroup& GetGroup() const = 0;
		virtual const void* GetSpecConstBuffer() const = 0;
	};
}
