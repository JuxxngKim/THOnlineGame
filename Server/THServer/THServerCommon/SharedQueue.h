#pragma once

namespace util
{
	template <typename T>
	class SharedQueue
	{
	private:
		std::mutex m_lock;
		std::queue<T> m_queue;

	public:
		SharedQueue() :
			m_lock{},
			m_queue{}
		{
		}

		virtual ~SharedQueue()
		{
			std::lock_guard<std::mutex> lck(m_lock);
			auto size = m_queue.size();
			for (int32_t i = 0; i < size; ++i)
			{
				m_queue.pop();
			}
		}

		void Push(T object)
		{
			std::lock_guard<std::mutex> guard(m_lock);
			m_queue.push(object);
		}

		T Pop()
		{
			std::lock_guard<std::mutex> guard(m_lock);
			if (m_queue.empty()) return {};

			auto data = m_queue.front();
			m_queue.pop();

			return data;
		}

		bool Empty()
		{
			std::lock_guard<std::mutex> guard(m_lock);
			return m_queue.empty();
		}

		size_t Size()
		{
			std::lock_guard<std::mutex> guard(m_lock);
			return m_queue.size();
		}
	};
}