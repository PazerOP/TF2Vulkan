#include "TF2Vulkan/Util/Macros.h"

void Util::LogFunctionCall(const char* fnName, const char* fnSig, const char* file, int line)
{
	Msg("[TF2Vulkan] %s\n", fnSig);
}

void Util::EnsureConditionFailed(const char* condition, const char* fnSig, const char* file, int line)
{
	Warning("[TF2Vulkan] ENSURE(%s) failed @ %s, %s:%i\n", condition, fnSig, file, line);
	assert(false);
	Error("[TF2Vulkan] ENSURE(%s) failed @ %s, %s:%i\n", condition, fnSig, file, line);
}
