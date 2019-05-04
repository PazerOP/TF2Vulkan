#pragma once

namespace Util
{
	namespace detail
	{
		template<typename T>
		struct DefaultMover
		{
			void PostMove(T& src) const
			{
				src = {};
			}
		};
	}
	template<typename T, typename Deleter, typename Mover = detail::DefaultMover<T>>
	class UniqueObject final : Deleter, Mover
	{
		using self_type = UniqueObject<T, Deleter>;
	public:
		explicit UniqueObject(T&& obj = {}, Deleter&& deleter = Deleter{}) :
			Deleter(std::move(deleter)),
			m_Obj(std::move(obj))
		{
		}
		UniqueObject(const UniqueObject&) = delete;
		UniqueObject& operator=(const UniqueObject&) = delete;
		UniqueObject(UniqueObject&& other) noexcept :
			Deleter(std::move(other)),
			Mover(std::move(other)),
			m_Obj(std::move(other.m_Obj))
		{
			get_mover().PostMove(other.m_Obj);
		}
		UniqueObject& operator=(UniqueObject&& other)
		{
			Destroy();
			get_deleter() = std::move(other.get_deleter());
			get_mover() = std::move(other.get_mover());

			m_Obj = std::move(other.m_Obj);
			get_mover().PostMove(other.m_Obj);

			return *this;
		}
		~UniqueObject() { Destroy(); }

		Deleter& get_deleter() { return *this; }
		const Deleter& get_deleter() const { return *this; }

		Mover& get_mover() { return *this; }
		const Mover& get_mover() const { return *this; }

		T& get() { return m_Obj; }
		const T& get() const { return m_Obj; }

		explicit operator T& () { return get(); }
		explicit operator const T& () const { return get(); }

	private:
		void Destroy()
		{
			get_deleter()(m_Obj);
		}

		T m_Obj;
	};
}
