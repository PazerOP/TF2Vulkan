#pragma once

#include <tier0/dbg.h>

#define NOT_IMPLEMENTED_FUNC_NOBREAK() \
	Warning("TF2Vulkan: %s in %s:%i not implemented\n", __FUNCSIG__, __FILE__, __LINE__);

#ifdef MATT_HAYNIE
#define NOT_IMPLEMENTED_FUNC() \
	NOT_IMPLEMENTED_FUNC_NOBREAK(); \
	__debugbreak();
#else
#define NOT_IMPLEMENTED_FUNC() NOT_IMPLEMENTED_FUNC_NOBREAK()
#endif
