#pragma once

namespace th
{
	class PacketDistributor;
	class PlayerExecutor
	{
	public:
		PlayerExecutor();
		virtual ~PlayerExecutor();

	public:
		void CreateThread(int32_t threadCnt);
		void Ready(const PTR<PacketDistributor>& distributor);
		bool IsComplete();
		void Exit();

	private:
		void Loop();

	private:
		std::mutex m_lck;
		std::condition_variable m_fieldWait;
		std::condition_variable m_logicWait;
		bool m_ready;
		std::atomic<int32_t> m_unitIdx;
		std::atomic<int32_t> m_completeCount;

		PTR<PacketDistributor> m_distributor;
		std::vector<PTR<std::thread>> m_threadPool;
		std::atomic<bool> m_active;
	};
}
