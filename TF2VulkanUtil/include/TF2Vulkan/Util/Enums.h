#pragma once

#include <type_traits>

namespace Util
{
	template<typename T>
	constexpr auto UValue(const T& val)
	{
#pragma warning(suppress : 4353)
		return std::underlying_type_t<T>(val);
	}

	namespace detail
	{
		template<typename T> struct EnumFlagOpsState : std::false_type {};

		template<typename T, typename = std::enable_if_t<EnumFlagOpsState<T>::value>>
		struct EnumFlagsWrapper
		{
			constexpr EnumFlagsWrapper(T val) : m_Value(std::underlying_type_t<T>(val)) {}

			constexpr operator bool() const { return !!m_Value; }
			constexpr operator T() const { return T(m_Value); }

			constexpr EnumFlagsWrapper<T> operator&(const EnumFlagsWrapper<T>& rhs) const
			{
				return EnumFlagsWrapper<T>(T(m_Value & rhs.m_Value));
			}
			constexpr EnumFlagsWrapper<T> operator|(const EnumFlagsWrapper<T>& rhs) const
			{
				return EnumFlagsWrapper<T>(T(m_Value | rhs.m_Value));
			}

			constexpr bool operator==(const T& rhs) const { return T(m_Value) == rhs; }
			constexpr bool operator!=(const T& rhs) const { return T(m_Value) != rhs; }

		private:
			std::underlying_type_t<T> m_Value;
		};
	}
}

#define ENABLE_ENUM_FLAG_OPS(enumType) \
	template<> struct ::Util::detail::EnumFlagOpsState<enumType> : std::true_type {};

#pragma push_macro("EFW")
#undef EFW
#define EFW ::Util::detail::EnumFlagsWrapper

template<typename T>
constexpr EFW<T> operator&(const T& lhs, const EFW<T>& rhs)
{
	return EFW{ lhs } &rhs;
}
template<typename T>
constexpr EFW<T> operator&(const T& lhs, const T& rhs)
{
	return EFW{ lhs } &EFW{ rhs };
}

template<typename T>
constexpr EFW<T> operator|(const T& lhs, const EFW<T>& rhs)
{
	return EFW{ lhs } | EFW{ rhs };
}
template<typename T>
constexpr EFW<T> operator|(const T& lhs, const T& rhs)
{
	return EFW{ lhs } | EFW{ rhs };
}

template<typename T>
constexpr T& operator|=(T& lhs, const EFW<T>& rhs)
{
	return lhs = (EFW{ lhs } | EFW{ rhs });
}
template<typename T>
constexpr T& operator|=(T& lhs, const T& rhs)
{
	return lhs = (EFW{ lhs } | EFW{ rhs });
}

template<typename T>
constexpr EFW<T> operator~(const T& v)
{
	return EFW<T>{ T(~(::Util::UValue(v))) };
}

#pragma pop_macro("EFW")
