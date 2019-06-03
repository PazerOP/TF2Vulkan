#pragma once

#include <type_traits>

namespace Util
{
	template<typename T> struct DirtyVarHandlerDefault
	{
		void operator()(T& stored, const T& newVal) const
		{
			if (stored != newVal)
				m_IsDirty = true;
		}
		void operator()(T& stored, T&& newVal) const
		{
			if (stored != newVal)
				m_IsDirty = true;
		}

		bool IsDirty() const { return m_IsDirty; }
		void SetDirty() { m_IsDirty = true; }

	private:
		bool m_IsDirty = true;
	};

	template<typename T, typename THandler = DirtyVarHandlerDefault<T>>
	class DirtyVar final
	{
	public:
		DirtyVar() = default;
		explicit DirtyVar(T&& val, THandler&& callback = THandler{}) :
			m_Var(std::move(val)), m_Handler(std::move(callback))
		{
		}
		DirtyVar(const DirtyVar<T>& other) : m_Var(other.m_Var) {}
		DirtyVar(DirtyVar<T>&& other) : m_Var(std::move(other.m_Var))
		{
			other.m_Handler.SetDirty();
		}

		DirtyVar& operator=(const DirtyVar<T>& other)
		{
			m_Handler(m_Var, other.m_Var);
			return *this;
		}
		DirtyVar& operator=(DirtyVar<T>&& other)
		{
			m_Handler(m_Var, std::move(other.m_Var));
			other.m_Handler.SetDirty();
			return *this;
		}

		bool IsDirty() const { return m_Handler.IsDirty(); }

		operator const T& () const { return m_Var; }
		const T* operator->() const { return &m_Var; }
		const T& Get() const { return m_Var; }

		T& Set(const T& value)
		{
			m_Handler(m_Var, value);
			return m_Var;
		}
		T& Set(T&& value)
		{
			m_Handler(m_Var, std::move(value));
			return m_Var;
		}

	private:
		T m_Var = T{};
		[[no_unique_address]] THandler m_Handler = THandler{};
	};

	template<typename T, typename T2>
	inline void SetDirtyVar(T& target, const T2& newVal, bool& dirtyVar)
	{
		if (target != newVal)
		{
			if constexpr (std::is_arithmetic_v<T> || std::is_arithmetic_v<T2>)
				Util::SafeConvert(newVal, target);
			else
				target = newVal;

			dirtyVar = true;
		}
	}

	/*template<typename T, typename T2, typename TIndex, size_t size>
	inline void SetDirtyVar(T(&targetArray)[size], const TIndex& index, const T2& newVal, bool& dirtyVar)
	{
		return SetDirtyVar(Util::at(targetArray, index), newVal, dirtyVar);
	}*/
	template<typename T, typename T2, typename TIndex>
	inline auto SetDirtyVar(T& container, const TIndex& index, const T2& newVal, bool& dirtyVar) ->
		decltype(Util::at(container, Util::SafeConvert<size_t>(index)), void())
	{
		return SetDirtyVar(Util::at(container, Util::SafeConvert<size_t>(index)), newVal, dirtyVar);
	}
}
