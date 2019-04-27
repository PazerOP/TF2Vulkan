#include "Threads.h"

#include <thread>

static std::thread::id s_MainThreadID = std::this_thread::get_id();

bool Util::IsMainThread()
{
	return std::this_thread::get_id() == s_MainThreadID;
}
