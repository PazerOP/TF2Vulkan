#pragma once

#include <vector>

namespace Util
{
	template<typename Tag>
	class AutoInitTagBase
	{
	protected:
		static void AutoInitAll();
		static void AutoShutdownAll();
	};

	template<typename Tag>
	class IAutoInit
	{
	protected:
		IAutoInit()
		{
			m_Instances.push_back(this);
		}
		~IAutoInit()
		{
			m_Instances.erase(std::find(m_Instances.begin(), m_Instances.end(), this));
		}

		static_assert(std::is_base_of_v<AutoInitTagBase<Tag>, Tag>);

		virtual void AutoInit() {};
		virtual void AutoShutdown() {};

	private:
		friend class AutoInitTagBase<Tag>;
		inline static std::vector<IAutoInit<Tag>*> m_Instances;
	};

	template<typename Tag>
	inline void AutoInitTagBase<Tag>::AutoInitAll()
	{
		for (auto inst : IAutoInit<Tag>::m_Instances)
			inst->AutoInit();
	}

	template<typename Tag>
	inline void AutoInitTagBase<Tag>::AutoShutdownAll()
	{
		for (auto inst : IAutoInit<Tag>::m_Instances)
			inst->AutoShutdown();
	}
}
