#include "interface/internal/IVulkanCommandBuffer.h"
#include "PixScope.h"

using namespace TF2Vulkan;

PixScope::PixScope(IVulkanCommandBuffer& buf) :
	m_CmdBuf(buf)
{
}

PixScope::~PixScope()
{
	m_CmdBuf.endDebugUtilsLabelEXT();
}
