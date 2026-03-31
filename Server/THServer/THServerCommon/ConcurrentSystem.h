#pragma once
#include "SharedQueue.h"
#include "ConcurrentActor.h"

namespace th
{
	class ConcurrentActorRef;
	class ConcurrentSystem : public std::enable_shared_from_this<ConcurrentSystem>
	{
	private:
		std::atomic<bool> m_active;

		std::recursive_mutex m_actorLock;
		std::unordered_map<ActorID_t, PTR<ConcurrentActor>> m_actors;

		util::SharedQueue<PTR<ConcurrentActor>> m_pendingJob;
		std::vector<PTR<std::thread>> m_threadPool;

		std::chrono::steady_clock::time_point m_metricUpdateTime;
		PTR<std::thread> m_metricThread;

	public:
		ConcurrentSystem();
		virtual ~ConcurrentSystem();

		ConcurrentSystem(const ConcurrentSystem&) = delete;
		ConcurrentSystem(const ConcurrentSystem&&) = delete;
		ConcurrentSystem& operator=(const ConcurrentSystem&) = delete;
		ConcurrentSystem& operator=(const ConcurrentSystem&&) = delete;

	public:
		void Start(int32_t threadCnt);
		void Exit();
		void Register(const PTR<ConcurrentActor>& actor);
		void Deregister(const ActorID_t id);
		bool RoutMessage(const PTR<ConcurrentActorRef>& receiver, const PTR<PacketWrapper>& msg);

	private:
		void Loop();
		void UpdateMetric();
	};
}