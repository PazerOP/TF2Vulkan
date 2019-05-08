#pragma once

#include <vector>

namespace Util
{
	namespace Buffer
	{
		template<typename T, typename TAllocator = std::allocator<std::byte>>
		void Put(std::vector<std::byte, TAllocator>& buf, const T& data)
		{
			static_assert(std::is_trivially_copyable_v<T>);

			const size_t oldSize = buf.size();
			buf.resize(oldSize + sizeof(data));
			memcpy(buf.data() + oldSize, &data, sizeof(data));
		}
	}
}
