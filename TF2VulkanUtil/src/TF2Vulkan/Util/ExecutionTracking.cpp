#include "TF2Vulkan/Util/ExecutionTracking.h"

void Util::LogFunctionCall(const char* fnName, const char* fnSig, const char* file, int line)
{
	Msg("[TF2Vulkan] %s\n", fnSig);
}
