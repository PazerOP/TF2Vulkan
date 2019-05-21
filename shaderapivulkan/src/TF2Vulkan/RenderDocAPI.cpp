#include "RenderDocAPI.h"
#include <TF2Vulkan/Util/interface.h>

#include <renderdoc_app.h>

using namespace TF2Vulkan;

static const auto& GetRenderDocAPI()
{
	static const auto s_API = []() -> RENDERDOC_API_1_4_0
	{
		const auto RENDERDOC_GetAPI = Util::FindProcAddress<pRENDERDOC_GetAPI>("renderdoc.dll", "RENDERDOC_GetAPI");
		if (!RENDERDOC_GetAPI)
			return {};

		RENDERDOC_API_1_4_0 retVal{};
		if (RENDERDOC_GetAPI && RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, reinterpret_cast<void**>(&retVal)))
			return retVal;

		return {};
	}();

	return s_API;
}

void RenderDoc::StartCapture()
{
	const auto sfc = GetRenderDocAPI().StartFrameCapture;
	if (!sfc)
		return;

	sfc(nullptr, nullptr);

	NOT_IMPLEMENTED_FUNC();
}
