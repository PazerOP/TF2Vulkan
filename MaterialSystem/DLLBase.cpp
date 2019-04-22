
#include <Windows.h>

extern "C" __declspec(dllexport) void* CreateInterface(const char* pName, int* pReturnCode)
{
	__debugbreak();
	OutputDebugStringA(pName);
	return nullptr;
}