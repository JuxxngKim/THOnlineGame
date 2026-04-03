#include "GamePch.h"
#include "OutGameLogicEventor.h"
#include "PacketWrapper.h"
#include "LogicUnitArchive.h"
#include "LoginPendingList.h"
#include "LogicUnit.h"
#include "WaitingSession.h"
#include "PlayerArchive.h"
#include "Player.h"
#include "ConcurrentCenter.h" 
#include "LoginSessionPlayer.h"
#include "NetworkManager.h"

namespace th
{
	OutGameLogicEventor::OutGameLogicEventor(const PTR<PacketCollector<PTR<PacketWrapper>>>& collector) : LogicEventor(collector)
		, m_lastServerInfoSyncTime{ util::TimeUtil::GetInstance().FindTickKSTMs() }
		, m_lastBiCurrentUserSyncTime{ util::TimeUtil::GetInstance().FindTickKSTMs() }
		, m_lastWaitingMessageInfoTime{ util::TimeUtil::GetInstance().FindTickKSTMs() }
		, m_nextPlayerCountSyncTime{ util::TimeUtil::GetInstance().FindTickKSTMs() + g_PlayerCountSyncMs }
		, m_nextServerAliveSyncTime{ util::TimeUtil::GetInstance().FindTickKSTMs() + g_AdventureServerAliveSyncMs }
		, m_loggingInArchive{ UNIQUE_NEW(LoginPendingList) } 
		, m_waitingSessions{ UNIQUE_NEW(WaitingSession) }
		, m_playerArchive{ UNIQUE_NEW(PlayerArchive) }
	{

		RegisterHandler(&OutGameLogicEventor::OnNetDisconnect, ELogicEvent::Prepare & ELogicEvent::Arrange);

		RegisterHandler(&OutGameLogicEventor::OnCALoginReq, ELogicEvent::Prepare);
		RegisterHandler(&OutGameLogicEventor::OnCAGetPlayerReq, ELogicEvent::Arrange);

		RegisterHandler(&OutGameLogicEventor::OnDALoginAck, ELogicEvent::Prepare & ELogicEvent::Arrange);
		RegisterHandler(&OutGameLogicEventor::OnDAEndofGameSessionAck, ELogicEvent::Prepare);

		m_waitingMessageID = {};
	}

	OutGameLogicEventor::~OutGameLogicEventor()
	{
		m_playerArchive = nullptr;
		m_loggingInArchive = nullptr;
		m_waitingSessions = nullptr;
		m_waitingMessageID.clear();
		m_loggedInPIDs.Clear();
	}

	void OutGameLogicEventor::Event(const int64_t& delta)
	{
		// 서버 완전기동 체크
		if (!m_waitingMessageID.empty())
		{
			if (m_lastWaitingMessageInfoTime + g_ServerReadyCheckTime <= delta)
			{
				m_lastWaitingMessageInfoTime = delta;

				std::string waitingMessageIDs{};
				for (const auto& messageID : m_waitingMessageID)
				{
					TH_LOG_INFO(0, 0, "waiting EMessageID : [%]", EMessageID_Name(messageID));
				}
			}
		}

		if (ServiceProfile::GetInstance().IsLocal()) return;

		{
			if (m_lastServerInfoSyncTime + g_ServerInfoSyncMs <= delta)
			{
				m_lastServerInfoSyncTime = delta;
				UpdateServerInfo();
			}

			if (m_lastBiCurrentUserSyncTime + g_biCurrentUserSyncMs <= delta)
			{
				m_lastBiCurrentUserSyncTime = delta;
				UpdateBICurrentUser();
			}
		}
	}

	void OutGameLogicEventor::UpdateServerInfoRedis(const int64_t& delta)
	{
		// TOOD
	}

	void OutGameLogicEventor::UpdatePlayerCount(const int64_t& delta)
	{
		// TODO
	}

	void OutGameLogicEventor::SyncAlive(const int64_t& delta)
	{
		// TODO
	}

	// process message
	void OutGameLogicEventor::OnNetDisconnect(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<NetDisconnect>& msg, const uint8_t& flag)
	{
		if (msg == nullptr) return;

		if (IsPrepareEvent(flag))
		{
			PrepareNetDisconnect(hostID, accountID, msg);
		}
		else if (IsArrangeEvent(flag))
		{
			ArrangeNetDisconnect(hostID, accountID, msg);
		}
	}

	void OutGameLogicEventor::PrepareNetDisconnect(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<NetDisconnect>& msg)
	{
		if (msg == nullptr) return;
	}

	void OutGameLogicEventor::ArrangeNetDisconnect(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<NetDisconnect>& msg)
	{
		if (msg == nullptr || m_playerArchive == nullptr) return;
		if (m_loggingInArchive == nullptr || m_waitingSessions == nullptr) return;

		if (m_loggingInArchive->IsExist(hostID))
		{
			m_waitingSessions->Unregister(hostID);

			const auto& enteringUnit = m_loggingInArchive->FindOwnerUnit(hostID);
			if (enteringUnit == nullptr) return;

			enteringUnit->Remove(hostID);
			LogicUnitArchive::GetInstance().Arrange(enteringUnit);

			m_loggingInArchive->Remove(hostID);
		}
		else if (m_playerArchive->IsSame(hostID, accountID))
		{
			const auto& player = m_playerArchive->FindPlayer(accountID);
			if (player == nullptr) return;

			player->Idle(hostID);
		}
	}

	void OutGameLogicEventor::OnCALoginReq(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<CALoginReq>& msg, const uint8_t& flag)
	{
		if (msg == nullptr || m_loggingInArchive == nullptr) return;

		if (!m_waitingMessageID.empty())
		{
			const auto& nak = NEW(ACCommonNak);
			nak->set_error(E_WAITING_ADVENTURE_SERVER_INIT);
			nak->set_successfulmsgid(AC_LOGIN_ACK);
			SendTo(hostID, nak->messageid(), nak);
			return;
		}

		if (m_loggingInArchive->IsExist(hostID))
		{
			TH_LOG_ERROR(hostID, 0, "duplicate login request");
			return;
		}

		const auto& player = NEW(LoginSessionPlayer, hostID, msg->pid());
		const auto& unit = LogicUnitArchive::GetInstance().EnterTo(player);
		if (unit == nullptr)
		{
			TH_LOG_ERROR(hostID, 0, "fail enter login field");
			m_loggingInArchive->Remove(hostID);
			return;
		}

		m_loggingInArchive->Add(player, unit);
		m_loggedInPIDs.Add()->assign(msg->pid());
	}

	void OutGameLogicEventor::OnCAGetPlayerReq(const HostID_t& hostID, const AccountUID_t& accountUID, const PTR<CAGetPlayerReq>& msg, const uint8_t& flag)
	{
		if (msg == nullptr || m_playerArchive == nullptr) return;

		const auto& player = m_playerArchive->FindPlayer(accountUID);
		if (player == nullptr) return;

		m_playerArchive->RegisterBroadCastingHostID(0, player->FindHostID());
	}


	void OutGameLogicEventor::OnDALoginAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DALoginAck>& msg, const uint8_t& flag)
	{
		if (msg == nullptr) return;

		if (IsPrepareEvent(flag))
		{
			PrepareLoginAck(hostID, accountID, msg);
		}
		else if (IsArrangeEvent(flag))
		{
			ArrangeLoginAck(hostID, accountID, msg);
		}
	}

	void OutGameLogicEventor::PrepareLoginAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DALoginAck>& msg)
	{
		if (msg == nullptr || m_playerArchive == nullptr || m_loggingInArchive == nullptr
			|| m_waitingSessions == nullptr) return;

		if (!m_loggingInArchive->IsExist(hostID)) return;
		if (m_playerArchive->IsExist(msg->accountid()))
		{
			if (!m_waitingSessions->RegisterWaiting(hostID, msg->accountid()))
			{
				const auto& nak = NEW(ACCommonNak);
				nak->set_error(E_DUPLICATE_CONNECTION);
				nak->set_successfulmsgid(AC_LOGIN_ACK);
				SendTo(hostID, nak->messageid(), nak);
				return;
			}

			return;
		}

		const auto& unit = m_loggingInArchive->FindOwnerUnit(hostID);
		if (unit == nullptr) return;

		unit->Remove(hostID);
		m_loggingInArchive->Remove(hostID);

		// 재연결시에 PlayerArchive에 이미 정리가 되어 있는 경우에 대한 처리
		if (msg->isreconnect() && !m_playerArchive->IsExist(msg->accountid()))
		{
			const auto& nak = NEW(ACCommonNak);
			nak->set_error(E_WRONG_PLAYER_DATA_CLIENT_RECONNECT);
			nak->set_successfulmsgid(AC_LOGIN_ACK);
			SendTo(hostID, nak->messageid(), nak);

			LogicUnitArchive::GetInstance().Arrange(unit);

			return;
		}

		const auto& player = NEW(Player);
		player->SetHostID(hostID);
		player->SetPID(msg->pid());
		player->SetAccountUID(msg->accountid());
		player->SetGameDBID(msg->gamedbid());
		player->SetName(msg->playername());
		player->SetPlayerLoginTime(msg->updatetime());
		player->SetLanguageID(msg->languageid());
		player->SetTotalPlayTime(msg->totalplaytime());
		player->RegisterDispatcher();
		unit->Add(player);
		m_playerArchive->Add(player, unit);
	}

	void OutGameLogicEventor::ArrangeLoginAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DALoginAck>& msg)
	{
		if (msg == nullptr || m_playerArchive == nullptr || m_loggingInArchive == nullptr
			|| m_waitingSessions == nullptr) return;

		//해당 hostID가 세션 대기중이라면 기존 Player의 HostId를 해당 hostID로 변경해준다.
		if (!m_loggingInArchive->IsExist(hostID)) return;
		if (!m_waitingSessions->IsWaiting(hostID, msg->accountid())) return;

		m_waitingSessions->Unregister(hostID);

		const auto& player = m_playerArchive->FindPlayer(msg->accountid());
		if (player == nullptr)
		{
			const auto& nak = NEW(ACCommonNak);
			nak->set_error(E_WRONG_PLAYER_DATA_CLIENT_RECONNECT);
			nak->set_successfulmsgid(AC_LOGIN_ACK);
			SendTo(hostID, nak->messageid(), nak);
			return;
		}

		if (player->IsEndOfGameSession())
		{
			const auto& nak = NEW(ACCommonNak);
			nak->set_error(E_WRONG_PLAYER_DATA_CLIENT_RECONNECT);
			nak->set_successfulmsgid(AC_LOGIN_ACK);
			SendTo(hostID, nak->messageid(), nak);
			return;
		}

		const auto& enteringUnit = m_loggingInArchive->FindOwnerUnit(hostID);
		if (enteringUnit != nullptr)
		{
			enteringUnit->Remove(hostID);
			LogicUnitArchive::GetInstance().Arrange(enteringUnit);
		}
		m_loggingInArchive->Remove(hostID);

		const auto& prevHostId = player->FindHostID();
		const auto& unit = m_playerArchive->FindOwnerUnit(accountID);
		if (unit == nullptr)
		{
			TH_LOG_ERROR(prevHostId, accountID, "not exist unit");
			return;
		}

		if (player->IsPlaying())
		{
			const auto& nak = NEW(ACCommonNak);
			nak->set_error(E_DUPLICATE_CONNECTION);
			nak->set_successfulmsgid(AC_LOGIN_ACK);
			SendTo(prevHostId, nak->messageid(), nak);
		}

		m_playerArchive->DeregisterBroadCastingHostID(0, prevHostId);

		std::optional<std::string> ip;
		if (msg->has_ip())
		{
			ip = msg->ip();
		}

		player->ChangeSession(hostID, msg->isreconnect(), msg->platformtype(), ip);
		player->SetLanguageID(msg->languageid());
	}

	void OutGameLogicEventor::UpdateServerInfo()
	{
		// TODO
	}

	void OutGameLogicEventor::UpdateBICurrentUser()
	{
		// TODO
	}

	void OutGameLogicEventor::OnDAEndofGameSessionAck(const HostID_t& hostID, const AccountUID_t& accountID
	                                                  , const PTR<DAEndofGameSessionAck>& msg, const uint8_t& flag)
	{
		if (m_playerArchive == nullptr) return;

		const auto& enteringUnit = m_playerArchive->FindOwnerUnit(accountID);
		if (enteringUnit == nullptr)
		{
			TH_LOG_ERROR(hostID, 0, "logic unit null");
			return;
		}

		enteringUnit->Remove(hostID);
		LogicUnitArchive::GetInstance().Arrange(enteringUnit);

		m_playerArchive->Remove(hostID, accountID);
	}
}