#include "interface/internal/IVBAllocTrackerInternal.h"

#include "TF2Vulkan/VertexFormat.h"
#include <TF2Vulkan/Util/interface.h>
#include <TF2Vulkan/Util/utlsymbol.h>

#include <unordered_map>

using namespace TF2Vulkan;

namespace
{
	struct VBInfo final
	{
		CUtlSymbolDbg m_Allocator;
		bool m_IsDynamic;
		size_t m_BufferSize;
		size_t m_VertexSize;
		VertexFormat m_Format;
	};

	class VBAllocTracker final : public IVBAllocTrackerInternal
	{
	public:
		void CountVB(void* buffer, bool isDynamic, int bufferSize, int vertexSize, VertexFormat_t fmt) override;
		void UnCountVB(void* buffer) override;
		bool TrackMeshAllocations(const char* allocatorName) override;

		CUtlSymbolDbg m_ActiveAllocator;
		std::unordered_map<void*, VBInfo> m_TrackedVBs;
	};
}

static VBAllocTracker s_VBAllocTracker;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(VBAllocTracker, IVBAllocTracker, VB_ALLOC_TRACKER_INTERFACE_VERSION, s_VBAllocTracker);

IVBAllocTrackerInternal& TF2Vulkan::g_VBAllocTracker = s_VBAllocTracker;

void VBAllocTracker::CountVB(void* buffer, bool isDynamic, int bufferSize, int vertexSize, VertexFormat_t fmt)
{
	LOG_FUNC();

	if (!m_ActiveAllocator)
	{
		assert(false);
		return;
	}

	m_TrackedVBs[buffer] = VBInfo
	{
		m_ActiveAllocator,
		isDynamic,
		Util::SafeConvert<size_t>(bufferSize),
		Util::SafeConvert<size_t>(vertexSize),
		VertexFormat(fmt)
	};
}

void VBAllocTracker::UnCountVB(void* buffer)
{
	LOG_FUNC();
	m_TrackedVBs.erase(buffer);
}

bool VBAllocTracker::TrackMeshAllocations(const char* allocatorName)
{
	LOG_FUNC();
	m_ActiveAllocator = allocatorName;
	return allocatorName && allocatorName[0];
}
