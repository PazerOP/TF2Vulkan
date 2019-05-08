#pragma once

#include <materialsystem/ivballoctracker.h>

namespace TF2Vulkan
{
	class IVBAllocTrackerInternal : public IVBAllocTracker
	{
	public:
	};

	extern IVBAllocTrackerInternal& g_VBAllocTracker;
}
