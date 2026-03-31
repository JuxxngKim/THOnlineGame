#include "GamePch.h"
#include "ConcurrentCenter.h"
#include "ConcurrentActorRef.h"
#include "PacketWrapper.h"
#include "ConcurrentActor.h"
#include "ConcurrentSystem.h"
#include "Configuration.h"
#include "OutGameService.h"
//#include "DBService.h" // TODO

namespace th
{
	ConcurrentCenter::ConcurrentCenter() : m_dbSystem{ nullptr }, m_systemMailbox{ nullptr }
	{
	}

	bool ConcurrentCenter::Init()
	{
		auto config = &Configuration::GetInstance();
		auto defaultCount = config->Get("Count", "DefaultDBActor").ToInt();

		if (defaultCount <= 0)
		{
			TH_LOG_ERROR(0, 0, "invalid count");
			return false;
		}

		// TODO
		//m_dbSystem = NEW(ConcurrentSystem);

		// TODO
		////NOTE:: 서버 내에 DB 요청을 위한 actor
		//const auto systemActor = NEW(DBService, 0, 0, [&](const PTR<PacketWrapper>& msg) { ConcurrentCenter::GetInstance()->SendToLogicService(msg); });
		//m_dbSystem->Register(systemActor);
		//m_systemMailbox = systemActor->Self();

		// TODO
		////AccountID가 발급되기 전 처리할 box생성, -1부터 시작.
		//for (int32_t i = 0; i < defaultCount; ++i)
		//{
		//	const auto actor = NEW(DBService, -(i + 1), 0, [&](const PTR<PacketWrapper>& msg) { ConcurrentCenter::GetInstance()->SendToLogicService(msg); });
		//	m_dbSystem->Register(actor);
		//	m_noAccountMailbox.push_back(actor->Self());
		//}

		return true;
	}

	bool ConcurrentCenter::Start()
	{
		if (m_dbSystem == nullptr) return false;

		auto config = &Configuration::GetInstance();
		auto cs = config->Get("ThreadCount", "DB");
		if (!cs.Valid()) return false;

		auto threadCount = 0;
		if (cs.ToLower() == "default") threadCount = std::thread::hardware_concurrency();
		else threadCount = cs.ToInt();

		m_dbSystem->Start(threadCount);

		return true;
	}

	void ConcurrentCenter::Exit()
	{
		if (m_dbSystem == nullptr) return;

		m_dbSystem->Exit();
		m_dbSystem = nullptr;
	}

	void ConcurrentCenter::RegisterDBActor(const PTR<ConcurrentActor>& actor)
	{
		if (actor == nullptr) return;

		if (m_dbSystem == nullptr)
		{
			TH_LOG_ERROR(0, 0, "db system is empty");
			return;
		}

		actor->SetMsgLimitMs(g_LogicAlarmLimitMs);
		m_dbSystem->Register(actor);
	}

	void ConcurrentCenter::DeregisterDBActor(const ActorID_t id)
	{
		if (m_dbSystem == nullptr) return;

		m_dbSystem->Deregister(id);
	}

	void ConcurrentCenter::SendToLogicService(const PTR<PacketWrapper>& msg)
	{
		//NOTE:: non thread safe
		if (msg == nullptr) return;

		OutGameService::GetInstance().Send(msg);
	}

	//NOTE::AccountID발급전, 게임내에서 DB조회해야할 경우 참조할 Actor
	void ConcurrentCenter::SendToSystemDB(const HostID_t& hostID, const PTR<PacketWrapper>& msg)
	{
		if (hostID == 0)
		{
			//NOTE::게임서버 자체 요청.
			if (m_systemMailbox == nullptr) return;

			m_systemMailbox->Send(msg);
			return;
		}

		if (m_noAccountMailbox.empty()) return;

		const auto idx = static_cast<int32_t>(hostID % m_noAccountMailbox.size());
		if (idx < 0 || m_noAccountMailbox.size() <= idx)
		{
			TH_LOG_ERROR(0, 0, "db system receiver is invalid idx : [idx:%]", idx);
			return;
		}

		if (m_noAccountMailbox[idx] == nullptr)
		{
			TH_LOG_ERROR(0, 0, "mail box is null");
			return;
		}

		m_noAccountMailbox[idx]->Send(msg);
	}
}
