#pragma once

#include <cassert>
#include <mutex>

namespace Util
{
	template<typename T>
	class MutexDbgWrapper
	{
	public:
		void lock()
		{
			m_Mutex.lock();

#ifdef _DEBUG
			if (m_LockCount++ == 0)
				m_OwningThread = std::this_thread::get_id();
#endif
		}

		void unlock()
		{
#if _DEBUG
			auto oldLock = m_LockCount--;
			assert(oldLock > 0);
			if (oldLock == 1)
				m_OwningThread = {};
#endif

			m_Mutex.unlock();
		}

		void assert_has_lock()
		{
#ifdef _DEBUG
			assert(m_OwningThread == std::this_thread::get_id());
#endif
		}

	private:
#ifdef _DEBUG
		std::thread::id m_OwningThread;
		uint32_t m_LockCount = 0;
#endif
		T m_Mutex;
	};

	using MutexDbg = MutexDbgWrapper<std::mutex>;
	using RecursiveMutexDbg = MutexDbgWrapper<std::recursive_mutex>;
}
