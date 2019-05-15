#include "Threads.h"

#include <thread>

static const auto& GetMainThreadID()
{
	static const std::thread::id s_MainThreadID = std::this_thread::get_id();
	return s_MainThreadID;
}

bool Util::IsMainThread()
{
	return std::this_thread::get_id() == GetMainThreadID();
}
