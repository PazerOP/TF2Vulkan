#pragma once

#include <memory>

namespace Util{ namespace memory
{
	struct FreeDeleter
	{
		template<typename T>
		void operator()(T* data) const
		{
			free(data);
		}
	};
} }
