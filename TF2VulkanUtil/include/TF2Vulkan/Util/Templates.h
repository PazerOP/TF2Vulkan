#pragma once

namespace Util
{
	template<typename T> struct make_dependent
	{
		using type = T;
	};

	template<typename T> using make_dependent_t = typename make_dependent<T>::type;
}
