#pragma once

namespace th
{
	class OutGameLogicEventor;
	class PlayerExecutor;
	class PacketDistributor;
	class PacketDualMap;
	class OutGameService : public Singleton<OutGameService>
	{
	public:
		OutGameService();
		virtual ~OutGameService() = default;
		OutGameService(const OutGameService&) = delete;
		OutGameService& operator=(const OutGameService&) = delete;

	public:
		void Start();
		void Send(const PTR<PacketWrapper>& msg);
		void Stop();
		bool IsQuit() const;
		bool IsCrash() const;

	private:
		void Execute();

	private:
		PTR<PacketDistributor> m_distributor;
		PTR<PlayerExecutor> m_executor;
		PTR<OutGameLogicEventor> m_eventor;
		PTR<std::thread> m_thread;
		PTR<PacketDualMap> m_msgBox;
		int64_t m_nextUpdateTime;
		std::atomic<bool> m_active;
		int64_t m_nextServiceCheckTime;
	};
}