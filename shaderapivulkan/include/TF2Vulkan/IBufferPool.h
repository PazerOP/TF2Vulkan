#pragma once

#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan
{
	class IBufferPool;

	class BufferPoolEntry final
	{
	public:
		constexpr BufferPoolEntry() = default;
		constexpr BufferPoolEntry(size_t size, size_t offset, IBufferPool& pool) :
			m_Size(size), m_Offset(offset), m_Pool(&pool)
		{
		}

		DEFAULT_STRONG_EQUALITY_OPERATOR(BufferPoolEntry);

		IBufferPool& GetPool() { assert(m_Pool); return *m_Pool; }
		const IBufferPool& GetPool() const { assert(m_Pool); return *m_Pool; }

		size_t GetOffset() const { assert(m_Pool); return m_Offset; }

		template<typename T> BufferPoolEntry& Update(const T& data);
		BufferPoolEntry& Update(const void* data, size_t size);

		operator bool() const { return !!m_Pool; }

	private:
		IBufferPool* m_Pool = nullptr;
		size_t m_Size = 0;
		size_t m_Offset = 0;
	};

	class IBufferPool
	{
	protected:
		~IBufferPool() = default;

	public:
		virtual BufferPoolEntry Create(size_t size) = 0;

		virtual void Update(const void* data, size_t size, size_t offset) = 0;

		// Helpers
		template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
		void Update(const BufferPoolEntry& buffer, const T& data)
		{
			return Update(&data, sizeof(data), buffer.GetOffset());
		}

		// Create and initialize
		BufferPoolEntry Create(size_t size, const void* initialData)
		{
			auto buf = Create(size);
			Update(initialData, size, buf.GetOffset());
			return buf;
		}
		template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
		BufferPoolEntry Create(const T& initialData)
		{
			return Create(sizeof(initialData), &initialData);
		}
	};

	inline BufferPoolEntry& BufferPoolEntry::Update(const void* data, size_t size)
	{
		assert(size <= m_Size);
		GetPool().Update(data, size, m_Offset);
		return *this;
	}

	template<typename T>
	inline BufferPoolEntry& BufferPoolEntry::Update(const T& data)
	{
		static_assert(!std::is_pointer_v<T>);
		Update(&data, sizeof(data));
		return *this;
	}
}
