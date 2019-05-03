#pragma once

#include <memory>

namespace Util
{
	class MemoryPool final
	{
	public:
		template<typename T, typename... Args> T& Allocate(Args&&... args)
		{
			auto obj = std::make_unique<MemoryObj<T>>(std::forward<Args>(args)...);

			T& realObj = obj.m_Obj;

			obj.m_Next = std::move(m_Head);
			m_Head = std::move(obj);

			return realObj;
		}

	private:
		struct IMemoryObj
		{
			virtual ~IMemoryObj() = default;
			std::unique_ptr<IMemoryObj> m_Next;
		};

		template<typename T>
		struct MemoryObj : IMemoryObj
		{
			template<typename... TArgs> MemoryObj(TArgs&& ... args) :
				m_Obj(std::forward<TArgs>(args)...)
			{
			}

			T m_Obj;
		};

		std::unique_ptr<IMemoryObj> m_Head;
	};
}
