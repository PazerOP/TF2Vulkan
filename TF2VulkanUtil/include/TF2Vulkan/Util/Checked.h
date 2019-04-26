#pragma once

#include <cassert>

namespace Util
{
	namespace detail
	{
		class CheckedBase
		{
		protected:
			constexpr CheckedBase() = default;

			void MarkChecked() const
			{
#ifdef _DEBUG
				m_Checked = true;
#endif
			}

			void EnsureChecked() const
			{
				assert(m_Checked);
			}

		private:
#ifdef _DEBUG
			mutable bool m_Checked = false;
#endif
		};
	}
	template<typename T>
	class CheckedPtr final : detail::CheckedBase
	{
	public:
		constexpr CheckedPtr() = default;
		constexpr CheckedPtr(T* ptr) : m_Ptr(ptr) {}

		operator bool() const
		{
			m_Checked = true;
			return (bool)m_Ptr;
		}

		operator T* () const
		{
			EnsureChecked();
			return m_Ptr;
		}

		T* operator->() const
		{
			EnsureChecked();
			return m_Ptr;
		}

	private:
		T* m_Ptr = nullptr;
	};

	template<typename T>
	class CheckedObj final : detail::CheckedBase
	{
	public:
		CheckedObj() = default;

		operator bool() const
		{
			MarkChecked();
			return (bool)m_Obj;
		}

		operator T& () const
		{
			EnsureChecked();
			return m_Obj;
		}

		T* operator->() const
		{
			EnsureChecked();
			return &m_Obj;
		}

	private:
		T m_Obj;
	};
}
