#include "CommonPch.h"
#include "ConcurrentSystem.h"
#include "ConcurrentActor.h"
#include "ConcurrentActorRef.h"
#include "ExitEventActor.h"
//#include "DBSessionPool.h" // TODO
//#include "DBSession.h" // TODO

namespace th
{
	ConcurrentSystem::ConcurrentSystem() :
		m_active{ false }, m_metricThread{ nullptr }
	{
	}

	ConcurrentSystem::~ConcurrentSystem()
	{
		Exit();
	}

	void ConcurrentSystem::Start(int32_t threadCnt)
	{
		m_active = true;
		for (int32_t i = 0; i < threadCnt; ++i)
		{
			const auto result = NEW(std::thread, [&]()
				{
					Loop();
				});

			m_threadPool.push_back(result);
		}

		if (m_metricThread == nullptr)
		{
			m_metricThread = NEW(std::thread, [&]
				{
					UpdateMetric();
				});
		}
	}

	void ConcurrentSystem::Loop()
	{
		// TODO
		//const auto type = { EDBSession::Arena, EDBSession::Game, EDBSession::Guild, EDBSession::Membership, EDBSession::TotalWar, EDBSession::Social };
		//auto session = NEW(DBSession, type);
		//DBSessionPool::GetInstance()->RegisterSession(session);

		while (m_active)
		{
			try
			{
				const auto actor = m_pendingJob.Pop();
				if (actor == nullptr)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}

				// TODO
				//actor->RegisterSession(session);
				actor->Execute();
				if (!actor->IsEmpty())
				{
					m_pendingJob.Push(actor);
				}
				else
				{
					if (!actor->CompareExchangeRunningFlag(true, false))
					{
						TH_LOG_ERROR(0, 0, "running flag abnormal behavior");
						m_pendingJob.Push(actor);
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			catch (const std::runtime_error& err)
			{
				TH_LOG_ERROR(0, 0, "ConcurrentSystem runtime error occurred : %", err.what());
			}
		}
	}

	void ConcurrentSystem::UpdateMetric()
	{
		while (m_active)
		{
			const auto& now = std::chrono::steady_clock::now();
			if (now < m_metricUpdateTime)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			m_metricUpdateTime += std::chrono::milliseconds(5000);
		}
	}

	void ConcurrentSystem::Exit()
	{
		m_active = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		auto exitEvent = NEW(ExitEventActor);
		for (const auto& handle : m_threadPool)
		{
			m_pendingJob.Push(exitEvent);
		}

		for (const auto& handle : m_threadPool)
		{
			if (handle->joinable()) handle->join();
		}

		if (m_metricThread != nullptr && m_metricThread->joinable()) m_metricThread->join();
	}

	void ConcurrentSystem::Register(const PTR<ConcurrentActor>& actor)
	{
		while (true)
		{
			ActorID_t id = actor->GetActorID();

			std::lock_guard<std::recursive_mutex> lck(m_actorLock);
			if (m_actors.contains(id)) continue;

			actor->SetSystem(shared_from_this(), id);
			m_actors.insert({ actor->GetActorID(), actor });
			break;
		}
	}

	void ConcurrentSystem::Deregister(const ActorID_t id)
	{
		std::lock_guard<std::recursive_mutex> lck(m_actorLock);
		const auto& it = m_actors.find(id);
		if (it == m_actors.end()) return;

		m_actors.erase(it);
	}

	bool ConcurrentSystem::RoutMessage(const PTR<ConcurrentActorRef>& receiver, const PTR<PacketWrapper>& msg)
	{
		std::lock_guard<std::recursive_mutex> lck(m_actorLock);
		auto it = m_actors.find(receiver->GetActorID());
		if (it == m_actors.end())
		{
			TH_LOG_INFO(0, 0, "actor not exist : [id:%]", receiver->GetActorID());
			return false;
		}

		const auto& actor = it->second;
		actor->Send(msg);
		if (actor->CompareExchangeRunningFlag(false, true))
		{
			m_pendingJob.Push(it->second);
		}

		return true;
	}
}
