#pragma once

#include "MutexWrapper.h"

namespace Util
{
	template<typename T, typename TMutex = std::recursive_mutex>
	class SynchronizedObject final
	{
		using WrappedMutex = MutexDbgWrapper<TMutex>;
	public:
		SynchronizedObject(const T& obj, WrappedMutex& mtx) :
			m_Object(obj), m_Mutex(&mtx)
		{
		}
		SynchronizedObject(T&& obj, WrappedMutex& mtx) :
			m_Object(std::move(obj)), m_Mutex(&mtx)
		{
		}

	private:
		using PointerDerefType = std::conditional_t<std::is_pointer_v<T>, T, T*>;
		using PointerDerefTypeC = std::conditional_t<std::is_pointer_v<T>, const T, const T*>;

	public:

		const T& get() const { m_Mutex->assert_has_lock(); return m_Object; }
		T& get() { m_Mutex->assert_has_lock(); return m_Object; }
		PointerDerefTypeC operator->() const
		{
			if constexpr (std::is_pointer_v<T>)
				return get();
			else
				return &get();
		}
		PointerDerefType operator->()
		{
			if constexpr (std::is_pointer_v<T>)
				return get();
			else
				return &get();
		}

		void lock() const { m_Mutex->lock(); }
		void unlock() const { m_Mutex->unlock(); }
		[[nodiscard]] auto lock_unique() const { return std::unique_lock(*m_Mutex); }

		[[nodiscard]] std::tuple<T, std::unique_lock<WrappedMutex>> locked() const
		{
			return { m_Object, lock_unique() };
		}

	private:
		T m_Object;
		WrappedMutex* m_Mutex;
	};
}
