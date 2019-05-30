#pragma once

namespace Util
{
	template<typename TBase1>
	struct CollapsingStruct : TBase1
	{
	};

	template<typename TBase1, typename... TBaseOthers>
	struct CollapsingStruct : TBase1, CollapsingStruct<TBaseOthers...>
	{
	};
}
