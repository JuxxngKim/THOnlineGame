#include "CommonPch.h"
#include "LogicEventor.h"
#include "PacketWrapper.h"
#include "LogicUnitArchive.h"
#include "PacketLogFilter.h"
#include "ProtoMemberAdapter.h"

namespace th
{
	LogicEventor::LogicEventor(const PTR<PacketCollector<PTR<PacketWrapper>>>& collector) : m_collector{ collector }, m_needShutdown{ false }
		, m_shutdownCheckSyncTime{ util::TimeUtil::GetInstance().FindTickKSTMs() + static_cast<int64_t>(g_DefaultShutdownCheckTimeSec * g_SecToMs) }
	{
		TH_LOG_INFO(0, 0, "LogicEventor start [%][%]", m_shutdownCheckSyncTime.load(), util::TimeUtil::GetInstance().FindTickKSTMs());
	}

	void LogicEventor::Collect(const std::deque<PTR<PacketWrapper>>& packets)
	{
		if (packets.empty()) return;

		for (const auto& msg : packets)
		{
			if (msg == nullptr) continue;

			if (IsPreparePacket(msg->MessageID))
			{
				m_prepareEvents.push_back(msg);
			}
			if (IsArrangePacket(msg->MessageID))
			{
				m_arrangeEvents.push_back(msg);
			}
		}
	}

	void LogicEventor::Prepare()
	{
		if (m_prepareEvents.empty()) return;

		for (const auto& msg : m_prepareEvents)
		{
			util::PacketLogFilter::Write(msg->HostID, msg->AccountUID, "logic eventor prepare", static_cast<EMessageID>(msg->MessageID), *msg->Msg.get());
			Dispatch(msg->HostID, msg->AccountUID, msg->MessageID, msg->Msg, ELogicEvent::Prepare);
		}

		m_prepareEvents.clear();
	}

	void LogicEventor::Arrange()
	{
		if (m_arrangeEvents.empty()) return;

		for (const auto& msg : m_arrangeEvents)
		{
			util::PacketLogFilter::Write(msg->HostID, msg->AccountUID, "logic eventor arrange", static_cast<EMessageID>(msg->MessageID), *msg->Msg.get());
			Dispatch(msg->HostID, msg->AccountUID, msg->MessageID, msg->Msg, ELogicEvent::Arrange);
		}

		m_arrangeEvents.clear();
	}

	bool LogicEventor::CheckQuit() const
	{
		if (m_needShutdown) TH_LOG_ERROR(0, 0, "checkQuit m_needShutdown = true");
		return m_needShutdown;
	}

	bool LogicEventor::CheckCrash() const
	{
		// NOTE : MembershipDB ServerInfo에 업데이트를 못하면 Crash
		bool result = m_shutdownCheckSyncTime.load() < util::TimeUtil::GetInstance().FindTickKSTMs();
		if (result)
		{
			TH_LOG_ERROR(0, 0, "[%][%]", m_shutdownCheckSyncTime.load(), util::TimeUtil::GetInstance().FindTickKSTMs());
		}
		return result;
	}

	bool LogicEventor::IsPreparePacket(const int32_t& msgId) const
	{
		return m_prepareMsgIds.contains(msgId);
	}

	bool LogicEventor::IsArrangePacket(const int32_t& msgId) const
	{
		return m_arrangeMsgIds.contains(msgId);
	}

	void LogicEventor::DistributeEventMsgId(const uint8_t& flag, const int32_t& msgId)
	{
		if (IsPrepareEvent(flag))
		{
			m_prepareMsgIds.insert(msgId);
		}
		if (IsArrangeEvent(flag))
		{
			m_arrangeMsgIds.insert(msgId);
		}
	}

	bool LogicEventor::IsPrepareEvent(const uint8_t& flag) const
	{
		uint8_t result = flag | ELogicEvent::Prepare;
		return result == ELogicEvent::Prepare;
	}

	bool LogicEventor::IsArrangeEvent(const uint8_t& flag) const
	{
		uint8_t result = flag | ELogicEvent::Arrange;
		return result == ELogicEvent::Arrange;

	}

	bool LogicEventor::Dispatch(const HostID_t& hostID, const AccountUID_t& accountID, const int32_t& messageID, const PTR<google::protobuf::Message>& message, const uint8_t& flag)
	{
		if (message == nullptr) return false;

		const auto it = m_handlers.find(messageID);
		if (it == m_handlers.end()) return false;

		(it->second)(hostID, accountID, message, flag);
		return true;
	}

	void LogicEventor::SendTo(const HostID_t& hostID, const int32_t& messageID, const PTR<google::protobuf::Message>& message)
	{
		network::NetworkManager::GetInstance().Send(hostID, messageID, message);
	}

	void LogicEventor::SendTo(std::unordered_set<HostID_t>& hostIDs, const int32_t& messageID, const PTR<google::protobuf::Message>& message)
	{
		network::NetworkManager::GetInstance().Send(hostIDs, messageID, message);
	}

	void LogicEventor::SetShutdownCheckSyncTime(const MDateTime& updateDate, const int64_t& checkTimeMs)
	{
		THDateTime updateSyncTime;
		ProtoMemberAdapter::ConvertMDateTimeToReDateTime(updateDate, updateSyncTime);
		m_shutdownCheckSyncTime = util::TimeUtil::GetInstance().ConvertTHDateTimeToKSTTimeMs(updateSyncTime) + max(checkTimeMs, g_ServerShutdownCheckMinTimeMS);
	}
}
