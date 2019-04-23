#include "TF2Vulkan/DebugTextureInfo.h"
#include "Util/Placeholders.h"

#include <materialsystem/idebugtextureinfo.h>

using namespace TF2Vulkan;

namespace
{
	class DebugTextureInfo : public IDebugTextureInfo
	{
	public:
		void EnableDebugTextureList(bool enable) override;
		void EnableGetAllTextures(bool enable) override;

		KeyValues* GetDebugTextureList() override;

		int GetTextureMemoryUsed(TextureMemoryType type) override;
		bool IsDebugTextureListFresh(int numFramesAllowed) override;
		bool SetDebugTextureRendering(bool enable) override;
	};
}

static DebugTextureInfo s_DebugTextureInfo;
IDebugTextureInfo* TF2Vulkan::GetDebugTextureInfo()
{
	return &s_DebugTextureInfo;
}

void DebugTextureInfo::EnableDebugTextureList(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

void DebugTextureInfo::EnableGetAllTextures(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
}

KeyValues* DebugTextureInfo::GetDebugTextureList()
{
	NOT_IMPLEMENTED_FUNC();
	return nullptr;
}

int DebugTextureInfo::GetTextureMemoryUsed(TextureMemoryType type)
{
	NOT_IMPLEMENTED_FUNC();
	return 0;
}

bool DebugTextureInfo::IsDebugTextureListFresh(int numFramesAllowed)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}

bool DebugTextureInfo::SetDebugTextureRendering(bool enable)
{
	NOT_IMPLEMENTED_FUNC();
	return false;
}
