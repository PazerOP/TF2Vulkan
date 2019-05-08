#include "interface/internal/IVBAllocTrackerInternal.h"

#include <TF2Vulkan/Util/interface.h>

using namespace TF2Vulkan;

namespace
{
	class VBAllocTracker final : public IVBAllocTrackerInternal
	{
	public:
		void CountVB(void* buffer, bool isDynamic, int bufferSize, int vertexSize, VertexFormat_t fmt) override { NOT_IMPLEMENTED_FUNC_NOBREAK(); }
		void UnCountVB(void* buffer) override { NOT_IMPLEMENTED_FUNC_NOBREAK(); }
		bool TrackMeshAllocations(const char* allocatorName) override;
	};
}

static VBAllocTracker s_VBAllocTracker;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VBAllocTracker, IVBAllocTracker, VB_ALLOC_TRACKER_INTERFACE_VERSION, s_VBAllocTracker);

IVBAllocTrackerInternal& TF2Vulkan::g_VBAllocTracker = s_VBAllocTracker;

bool VBAllocTracker::TrackMeshAllocations(const char* allocatorName)
{
	NOT_IMPLEMENTED_FUNC_NOBREAK();
	return false;
}
