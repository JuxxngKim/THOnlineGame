#include "CommonPch.h"
#include "ConcurrentActor.h"
#include "ConcurrentSystem.h"
#include "ConcurrentActorRef.h"
#include "MessageDelayAlarm.h"

namespace th
{
	ConcurrentActor::ConcurrentActor(int32_t limitMs) : m_id{ 0 }, m_self{ nullptr }, m_running{ false }, m_msgLimitMs{ limitMs }
	{
	}

	void ConcurrentActor::SetSystem(const PTR<ConcurrentSystem>& system, const ActorID_t& actorID)
	{
		m_id = actorID;
		m_self = NEW(ConcurrentActorRef, m_id, system);
	}

	void ConcurrentActor::Execute()
	{
		Update();

		Prepare();
		for (auto i = 0; i < MessageProcCount; ++i)
		{
			auto msg = m_messageBox.Pop();
			if (msg == nullptr) continue;

			MessageDelayAlarm alarm{ msg->AccountUID, msg->HostID, msg->MessageID, m_msgLimitMs };
			Run(msg);
		}
		Arrange();
	}

	void ConcurrentActor::SetMsgLimitMs(const int32_t ms)
	{
		m_msgLimitMs = ms;
	}

	PTR<ConcurrentActorRef> ConcurrentActor::Self()
	{
		return m_self;
	}

	bool ConcurrentActor::IsEmpty()
	{
		return m_messageBox.Empty();
	}

	bool ConcurrentActor::CompareExchangeRunningFlag(bool expected, bool change)
	{
		return m_running.compare_exchange_strong(expected, change);
	}
}
