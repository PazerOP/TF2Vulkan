#pragma once

#include <TF2Vulkan/Util/std_compare.h>

namespace TF2Vulkan
{
	class IUniformBufferPool;

	class UniformBuffer final
	{
	public:
		constexpr UniformBuffer() = default;
		constexpr UniformBuffer(size_t offset, IUniformBufferPool& pool) :
			m_Offset(offset), m_Pool(&pool)
		{
		}

		DEFAULT_STRONG_EQUALITY_OPERATOR(UniformBuffer);

		IUniformBufferPool& GetPool() { assert(m_Pool); return *m_Pool; }
		const IUniformBufferPool& GetPool() const { assert(m_Pool); return *m_Pool; }

		size_t GetOffset() const { assert(m_Pool); return m_Offset; }

		template<typename T> UniformBuffer& Update(const T& data);

		operator bool() const { return !!m_Pool; }

	private:
		IUniformBufferPool* m_Pool = nullptr;
		size_t m_Offset = 0;
	};

	class IUniformBufferPool
	{
	protected:
		~IUniformBufferPool() = default;

	public:
		virtual size_t GetChildBufferSize() const = 0;
		virtual size_t GetChildBufferCount() const = 0;
		virtual UniformBuffer Create() = 0;

		virtual void Update(const void* data, size_t size, size_t offset) = 0;

		// Helpers
		template<typename T> void Update(const UniformBuffer& buffer, const T& data)
		{
			return Update(&data, sizeof(data), buffer.GetOffset());
		}
	};

	template<typename T>
	inline UniformBuffer& UniformBuffer::Update(const T& data)
	{
		GetPool().Update(*this, data);
		return *this;
	}
}
