#pragma once
#include <deque>

namespace util
{
	template<typename T>
	class DualQueue
	{
	private:
		std::recursive_mutex m_lock{};
		bool usedMain{ false };
		std::deque<T> m_mainQueue{};
		std::deque<T> m_subQueue{};

	public:
		DualQueue()
		{
		}

		virtual ~DualQueue()
		{
			std::lock_guard<std::recursive_mutex> guard(m_lock);

			usedMain = false;
			m_mainQueue.clear();
			m_subQueue.clear();
		}

	public:
		void Push(T object)
		{
			std::lock_guard<std::recursive_mutex> guard(m_lock);

			if (usedMain)
			{
				m_mainQueue.push_back(object);
			}
			else
			{
				m_subQueue.push_back(object);
			}
		}

		std::deque<T>& Pop()
		{
			std::lock_guard<std::recursive_mutex> guard(m_lock);

			if (usedMain)
			{
				usedMain = false;

				return m_mainQueue;
			}
			else
			{
				usedMain = true;

				return m_subQueue;
			}
		}

		bool Empty()
		{
			std::lock_guard<std::recursive_mutex> guard(m_lock);

			if (usedMain)
			{
				//usedMain = false;
				return m_mainQueue.empty();
			}
			else
			{
				//usedMain = true;
				return m_subQueue.empty();
			}
		}

		int32_t Size()
		{
			// note : metric에서 size만 가지고 가기 위해서 lock을 의도적으로 걸지 않고 가져간다.
			return static_cast<int32_t>(m_mainQueue.size() + m_subQueue.size());
		}
	};
}
