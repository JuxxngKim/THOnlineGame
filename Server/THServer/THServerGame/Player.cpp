#include "GamePch.h"
#include "Player.h"
#include "LoginDataLoader.h"
#include "ConcurrentCenter.h"
#include "LogKeyGenerator.h"
#include "MessageDelayAlarm.h"
#include "PacketDistributor.h"
#include "PacketLogFilter.h"
#include "ProtoMemberAdapter.h"
//#include "DuplicateHostManager.h" // TODO

namespace th
{
	Player::Player()
		: SessionPlayer{ 0 }
		, m_pid{ "" }
		, m_name{ "" }
		, m_status{ EPlayerStatus::None }
		, m_lv{ 0 }
		, m_exp{ 0 }
		, m_gameDBID{ 0 }
		, m_idleTime{ 0 }
		, m_chattingChannelID{ 0 }
		, m_playerLoginTime{}
		, m_lastUpdateTimeMsUnion{ {ELastUpdateTimeIndex::Frequency, util::TimeUtil::GetInstance().FindTickKSTMs()},
								{ELastUpdateTimeIndex::ShortTerm, util::TimeUtil::GetInstance().FindTickKSTMs()},
								{ELastUpdateTimeIndex::LongTerm, util::TimeUtil::GetInstance().FindTickKSTMs()} }


		, m_reserveNewNReturnPlayerProcess{ false }
		, m_dbChannel{ nullptr }
		, m_dbDataLoader{ UNIQUE_NEW(LoginDataLoader) }
		, m_blockDuplicatePacket{ NEW(BlockDuplicatePacket) }
		, m_loginProcessTime{ 0 }
		, m_requestLoginData{ false }
		, m_isReconnect{ false }
		, m_languageID{ ELanguageCode::None }
		, m_totalPlayTimeMin{ 0 }
		, m_authenticated{ false }
		, m_loginExpireTimeMS{ util::TimeUtil::GetInstance().FindTickKSTMs() }
		, m_forceDisconnectionStatus{ EPlayerDisconnectionStatus::None }
		, m_isForceLogin{ false }
		, m_loginPlatform{ EPlatformType::NONE }
		, m_loginIP{ "" }
		, m_isFirstLogin{ false }
		, m_forceDisconnect{ false }
	{
		//NOTE:: client -> server
		RegisterHandler(this, &Player::OnNetDisconnect);
		RegisterHandler(this, &Player::OnCAGetPlayerReq);

		//NOTE:: DB -> Game
		RegisterHandler(this, &Player::OnDALoginAck);
		RegisterHandler(this, &Player::OnDAPlayerInfoAck);
		RegisterHandler(this, &Player::OnDAEndofGameSessionAck);
		//CommonMetric::GetInstance().IncrementObject("player");
	}

	Player::~Player()
	{
		//CommonMetric::GetInstance().DecrementObject("player");
	}

	void Player::Clear()
	{
		m_pid = "";
		m_name = "";
		m_lv = 0;
		m_exp = 0;
		m_gameDBID = 0;
		m_idleTime = 0;
		m_chattingChannelID = 0;
		m_playerLoginTime = {};
		m_languageID = ELanguageCode::None;
		m_totalPlayTimeMin = 0;
		m_lastUpdateTimeMsUnion.clear();

		// NOTE(jwoh) ConcurrentSystem을 들고 있어 따로 Clear를 호출하지 않도록한다.
		m_dbChannel = nullptr;

		m_forceDisconnectionStatus = EPlayerDisconnectionStatus::None;

		if (m_dbDataLoader != nullptr)
		{
			m_dbDataLoader->Clear();
			m_dbDataLoader = nullptr;
		}

		if (m_blockDuplicatePacket != nullptr)
		{
			m_blockDuplicatePacket->Clear();
			m_blockDuplicatePacket = nullptr;
		}

		m_syncLoginDatas.clear();

		m_loginProcessTime = 0;
		m_requestLoginData = false;
		m_isReconnect = false;
		m_authenticated = false;
		m_loginExpireTimeMS = 0;

		for (const auto& dispatcher : m_dispatchers)
		{
			if (dispatcher == nullptr) continue;
			dispatcher->Clear();
		}
		m_dispatchers.clear();
		m_reserveNewNReturnPlayerProcess = false;
		m_isFirstLogin = false;
		m_forceDisconnect = false;
	}

	void Player::RegisterDispatcher()
	{
		//m_dispatchers.insert(NEW(UnexpectedRaidDispatcher, shared_from_this()));
	}

	void Player::Execute(const PTR<PacketDistributor>& distributor)
	{
		if (distributor == nullptr || m_blockDuplicatePacket == nullptr || m_dbDataLoader == nullptr) return;

		Update();

		const auto& queue = distributor->Pick(FindAccountUID());
		for (const auto& packet : queue)
		{
			const auto messageID = static_cast<EMessageID>(packet->MessageID);
			const bool isDbPacket = ProtoMemberAdapter::IsDBPacket(messageID);

			MessageDelayAlarm alram{ FindAccountUID(), FindHostID(), messageID, g_LogicAlarmLimitMs };

			// NOTE: 호스트가 다른데 클라이언트 패킷인 경우에는 넘어간다.
			if (!isDbPacket && packet->HostID != FindHostID()) continue;

			/*
			 * NOTE : 로그가 남고 있어서 해당 부분 하드코딩 처리하고 이후에 따로 수정하자.
			 * NOTE : DALoginAck에서 호스트가 다른 경우에는 처리하지 않기 위해서 리턴.
			 */
			if (isDbPacket && packet->HostID != FindHostID() && messageID == DA_LOGIN_ACK) continue;

			util::PacketLogFilter::Write(packet->HostID, FindAccountUID(), "player execute packet", messageID, *packet->Msg.get());

			// Client의 요청인 경우에는 이미 요청 받은 내역이 있는지 체크해준다.
			if (m_blockDuplicatePacket->IsProcessing(messageID))
			{
				SendNakToHost(packet->HostID, E_DUPLICATE_CLIENT, messageID);
				continue;
			}

			m_blockDuplicatePacket->CheckDuplicate(messageID);
			if (g_MaxDuplicatePacketCount <= m_blockDuplicatePacket->FindDuplicateCount())
			{
				SendNakToHost(packet->HostID, E_DUPLICATE_CLIENT, messageID);
				TH_LOG_ERROR(FindHostID(), FindAccountUID(), "ForceDisconnect g_MaxDuplicatePacketCount, messageID:%", messageID);
				ForceDisconnect(FindHostID());
				m_blockDuplicatePacket->ResetDuplicateCount();
				return;
			}

			//NOTE::종료 처리중에 클라이언트 요청 패킷은 무시
			if (IsEndOfGameSession() && !isDbPacket) continue;

			// 초기 DB 정보 로딩이 완료되기 전에 클라이언트에서 받은 패킷은 제외함.
			if (!isDbPacket && m_requestLoginData && !m_dbDataLoader->IsAllCompleted())
			{
				SendNakToHost(packet->HostID, E_PLAYER_LOADING_NOT_COMPLETED, messageID);
				continue;
			}

			TL_ProcessingMsgID = packet->MessageID;
			Dispatch(packet->HostID, packet->MessageID, packet->Msg);

			// DB 패킷인 경우에는 DB 요청 인지 체크해서 풀어준다.
			if (isDbPacket)
			{
				m_blockDuplicatePacket->PacketComplete(packet->BlockKey);
				SyncDataFromClient();
			}
		}

		m_blockDuplicatePacket->ResetDuplicateCount();

		// GetPlayer 정보를 보내는 로직
		SyncPlayerDataToClient();
	}

	void Player::Idle(const HostID_t& hostID)
	{
		if (hostID != m_hostID) return;
		if (IsEndOfGameSession() || IsIdle()) return;

		m_status = EPlayerStatus::Idle;
		m_idleTime = util::TimeUtil::GetInstance().FindTickKSTMs();
		const auto& totalPlayTimeMin = FindTotalPlayTimeMin();

		// TODO
	/*	const auto dbReq = NEW(ADWriteLogLogoutNoti);
		dbReq->set_TotalPlayTimeMin(totalPlayTimeMin);
		dbReq->set_LogKey(IssueLogKey(dbReq->messageid()));
		ProtoMemberAdapter::ConvertReDateTimeToMDateTime(util::TimeUtil::GetInstance().FindKSTDate(), dbReq->mutable_LastUpdateTime());
		dbReq->set_NeedToSetIdleStatus(true);
		SendToDB(dbReq, NEW(NullMessage));*/
	}

	void Player::SetPlay(const HostID_t& hostID, const AccountUID_t& accountUID)
	{
		m_status = EPlayerStatus::Play;
		network::NetworkManager::GetInstance().RegisterHostValue(hostID, accountUID);
	}

	void Player::Update()
	{
		if (IsEndOfGameSession()) return;

		const auto curTimeMs = util::TimeUtil::GetInstance().FindKSTMs();
		const auto tickTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs();
		if (tickTimeMs < m_lastUpdateTimeMsUnion[ELastUpdateTimeIndex::Frequency] + g_PlayerFrequencyUpdateMs) return;

		//NOTE::로그인 데이터 로드와 별개로 무조건 돌아야함.
		UpdateEndOfGameSession();
		if (IsEndOfGameSession()) return;

		// TODO
		//// ChangeSession에 대한 HostID처리는 DBLoader와 상관없이 돌아야한다.
		//if (m_duplicateHostManager != nullptr)
		//{
		//	m_duplicateHostManager->Update();
		//}

		if (m_dbDataLoader != nullptr && !m_dbDataLoader->IsAllCompleted())
		{
			// Play 이전에 getPlayer가 가면 안됨
			if (!IsPlaying()) return;

			// 강제 로그인은 따로 CAGetPlayerReq가 오지 않음
			GetPlayerReq();
			m_isForceLogin = false;
			return;
		}

		FrequencyUpdate(curTimeMs);

		if (m_lastUpdateTimeMsUnion[ELastUpdateTimeIndex::ShortTerm] + g_PlayerShortTermUpdateMs <= tickTimeMs)
		{
			ShortTermUpdate(curTimeMs);
			m_lastUpdateTimeMsUnion[ELastUpdateTimeIndex::ShortTerm] = tickTimeMs;
		}

		if (m_lastUpdateTimeMsUnion[ELastUpdateTimeIndex::LongTerm] + g_PlayerLongTermUpdateMs <= tickTimeMs)
		{
			LongTermUpdate(curTimeMs);
			m_lastUpdateTimeMsUnion[ELastUpdateTimeIndex::LongTerm] = tickTimeMs;
		}

		m_lastUpdateTimeMsUnion[ELastUpdateTimeIndex::Frequency] = tickTimeMs;
	}

	/// <summary>
	/// 상시 업데이트
	/// NOTE(jwoh) 주로 메모리 초기화 정도의 업데이트
	/// </summary>
	void Player::FrequencyUpdate(const int64_t& curTimeMs)
	{
	}

	/// <summary>
	/// 짧은 주기 업데이트
	/// NOTE(jwoh) DB 부하가 우려되나 초단위 실시간을 보장해야하는 업데이트
	/// </summary>
	void Player::ShortTermUpdate(const int64_t& curTimeMs)
	{
	}

	/// <summary>
	/// 긴 주기 업데이트
	/// NOTE(jwoh) 컨텐츠가 초단위 실시간을 보장 하지 않아도 되며, DB 부하가 우려되는 업데이트
	/// </summary>
	void Player::LongTermUpdate(const int64_t& curTimeMs)
	{
	}

	void Player::UpdateEndOfGameSession()
	{
		if (!IsIdle()) return;

		//m_idleTime = util::TimeUtil::GetInstance().FindTickKSTMs();

		auto exitIdleTimeSpan = g_ForceDisconnectMs;
		if (m_forceDisconnect)
		{
			exitIdleTimeSpan = g_ForceDisconnectMs;
		}

		const auto curTime = util::TimeUtil::GetInstance().FindTickKSTMs();
		if (curTime < m_idleTime + exitIdleTimeSpan) return;

		EndOfGameSession();
	}

	void Player::SyncPlayerDataToClient()
	{
		if (m_dbDataLoader == nullptr) return;

		std::set<ELoginData> loadCompleteDatas = m_dbDataLoader->FindCompleteDatas();
		if (loadCompleteDatas.empty()) return;
		if (loadCompleteDatas.size() == m_syncLoginDatas.size()) return;
		if (!m_requestLoginData) return;

		// memo : player정리 단계에서는 아래 처리가 필요없을듯 하다.
		if (IsEndOfGameSession()) return;

		int64_t buildStartMs = util::TimeUtil::GetInstance().FindTickKSTMs();
		for (const auto& loginData : loadCompleteDatas)
		{
			const auto& findIter = m_syncLoginDatas.find(loginData);
			if (findIter != m_syncLoginDatas.end()) continue;

			m_syncLoginDatas.insert(loginData);

			switch (static_cast<ELoginData>(loginData))
			{
			case ELoginData::Player:
			{
				const auto noti = NEW(ACGetPlayerInfoNoti);
				noti->set_accountlevel(FindLevel());
				noti->set_accountexp(FindExp());
				SendToHost(noti);
				break;
			}
			default:
			{
				TH_LOG_ERROR(m_hostID, m_accountUID, "none Elogindata check[%]", static_cast<int32_t>(loginData));
				break;
			}
			}
		}

		const auto buildEndMs = util::TimeUtil::GetInstance().FindTickKSTMs();

		// 모두 완료가 되었을때 보내준다.
		if (static_cast<size_t>(ELoginData::Max) == m_syncLoginDatas.size())
		{
			auto ack = NEW(ACGetPlayerAck);
			SendToHost(ack);

			const auto gap = util::TimeUtil::GetInstance().FindTickKSTMs() - m_loginProcessTime;
			if (m_loginProcessTime <= 0)
			{
				TH_LOG_ERROR(m_hostID, m_accountUID, "login time is zero : [%]", gap);
			}
			if (5000 < gap)
			{
				const auto& loadHistory = m_dbDataLoader->FindLoadTimeMs();
				std::string history{};
				int64_t elapsedTimeTotalMs = 0;
				for (const auto& [loginDataType, elapsedTimeMs] : loadHistory)
				{
					history.append(std::to_string(static_cast<int32_t>(loginDataType)));
					history.append("-");
					history.append(std::to_string(elapsedTimeMs));
					history.append(",");
					elapsedTimeTotalMs += elapsedTimeMs;
				}

				TH_LOG_ERROR(m_hostID, m_accountUID, "over login time : [total:%, build:%, dbLoad:%]", gap, buildEndMs - buildStartMs, history);
			}
			m_loginProcessTime = 0;

			//SyncWorldRaidSeasonSchedule();
		}
	}

	void Player::GetPlayerReq()
	{
		if (m_dbDataLoader == nullptr) return;

		if (m_requestLoginData)
		{
			TH_LOG_ERROR(FindHostID(), FindAccountUID(), "duplicate GetPlayerReq call.");
			return;
		}

		// GetPlayerReq가 호출되는 시점에는 상태가 EPlayerStatus::Play로 변경되어 있다.
		if (!IsPlaying())
		{
			SendNakToHost(FindHostID(), E_WRONG_PLAYER_DATA_CLIENT_RECONNECT, AC_GET_PLAYER_ACK);
			return;
		}

		m_loginProcessTime = util::TimeUtil::GetInstance().FindTickKSTMs();
		m_requestLoginData = true;
		/* 최초 로그인의 경우에만 m_dbDataLoader를 호출하고 나머진 SyncPlayerDataToClient()를 통해서 보낸다.*/
		if (!m_dbDataLoader->IsAllCompleted())
		{
			m_dbDataLoader->Start(m_dbChannel);
		}
	}

	void Player::SetHostID(const HostID_t& hostID)
	{
		m_hostID = hostID;
		if (m_dbDataLoader != nullptr) m_dbDataLoader->SetHostID(hostID);
	}

	void Player::SetPID(const std::string& pid)
	{
		m_pid = pid;
		if (m_dbDataLoader != nullptr) m_dbDataLoader->SetPID(pid);
	}

	const std::string& Player::FindPID() const
	{
		return m_pid;
	}

	void Player::SetAccountUID(const AccountUID_t& accountUID)
	{
		m_accountUID = accountUID;
		if (m_dbDataLoader != nullptr) m_dbDataLoader->SetAccountUID(accountUID);
	}

	void Player::SetName(const std::string& name)
	{
		m_name = name;
	}

	const std::string& Player::FindName() const
	{
		return m_name;
	}

	void Player::SetGameDBID(const int32_t& id)
	{
		m_gameDBID = id;
		if (m_dbDataLoader != nullptr) m_dbDataLoader->SetGameDBID(id);
	}

	int32_t Player::FindGameDBID() const
	{
		return m_gameDBID;
	}

	void Player::SetDBChannel(const PTR<Mailbox_t>& channel)
	{
		if (channel == nullptr) return;

		m_dbChannel = channel;
	}

	int32_t Player::FindLevel() const
	{
		return m_lv;
	}

	int32_t Player::FindExp() const
	{
		return m_exp;
	}

	void Player::SetLevel(const int32_t& level)
	{
		m_lv = level;
	}

	void Player::SetPlayerLoginTime(const MDateTime& loginTime)
	{
		ProtoMemberAdapter::ConvertMDateTimeToReDateTime(loginTime, m_playerLoginTime);
		if (m_dbDataLoader != nullptr) m_dbDataLoader->SetLoginDateTime(m_playerLoginTime);
	}

	void Player::SetLanguageID(const int32_t& languageID)
	{
		m_languageID = static_cast<ELanguageCode>(languageID);
	}

	void Player::SetTotalPlayTime(const int32_t& totalPlayTime)
	{
		m_totalPlayTimeMin = totalPlayTime;
	}

	void Player::SetForceLogin(const bool& isForceLogin)
	{
		m_isForceLogin = isForceLogin;
	}

	int32_t Player::FindTotalPlayTimeMin() const
	{
		const auto& curDate = util::TimeUtil::GetInstance().FindKSTDate();
		const auto& curPlayTimeSec = util::TimeUtil::GetInstance().FindIntervalSeconds(m_playerLoginTime, curDate);
		return m_totalPlayTimeMin + static_cast<int32_t>(curPlayTimeSec / 60);	// 로그인한 이후의 진행시간도 더해서 준다.
	}

	PTR<BlockDuplicatePacket> Player::FindBlockDuplicatePacket()
	{
		return m_blockDuplicatePacket;
	}

	ELanguageCode Player::FindLanguageID() const
	{
		return m_languageID;
	}

	void Player::EndOfGameSession()
	{
		// Idle상태일때 한번만 호출되도록 한다.
		if (!IsIdle()) return;

		m_status = EPlayerStatus::EndofGameSession;

		// Memo : 여기서 로그아웃 전 동기화 처리
		THDateTime lastUpdateTime = util::TimeUtil::GetInstance().FindKSTDate();

		// Memo : DB 처리의 마무리 처리여서 뒤로는 DB 처리가 있으면 안된다.
		const auto dbReq = NEW(ADEndofGameSessionReq);
		dbReq->set_logkey(IssueLogKey(dbReq->messageid()));
		ProtoMemberAdapter::ConvertReDateTimeToMDateTime(lastUpdateTime, dbReq->mutable_lastupdatetime());
		SendToDB(dbReq, NEW(NullMessage));
	}

	bool Player::IsEndOfGameSession() const
	{
		return m_status == EPlayerStatus::EndofGameSession;
	}

	bool Player::IsIdle() const
	{
		return m_status == EPlayerStatus::Idle;
	}

	bool Player::IsPlaying() const
	{
		return m_status == EPlayerStatus::Play;
	}

	void Player::ChangeSession(const HostID_t& newHostID, const bool& isReconnect, const EPlatformType& loginPlatform, const std::optional<std::string>& loginIP)
	{
		const auto updateTime = util::TimeUtil::GetInstance().FindKSTDate();
		const auto& totalPlayTimeMin = FindTotalPlayTimeMin();

		if (IsPlaying())
		{
			// TODO
			//const auto dbReq = NEW(ADWriteLogLogoutNoti);
			//dbReq->set_TotalPlayTimeMin(totalPlayTimeMin);
			//dbReq->set_LogKey(IssueLogKey(dbReq->messageid()));
			//ProtoMemberAdapter::ConvertReDateTimeToMDateTime(updateTime, dbReq->mutable_LastUpdateTime());

			//// 네트워크 끊겼다가 다시 접속했을떄는 자리비움 상태로 변경하지 않는다.
			//dbReq->set_NeedToSetIdleStatus(false);
			//SendToDB(dbReq, NEW(NullMessage));

			// TODO
			//m_duplicateHostManager->ChangeSession(FindHostID());
		}

		SetHostID(newHostID);
		SetPlay(newHostID, m_accountUID);
		m_syncLoginDatas.clear();

		const auto ack = NEW(ACLoginAck);
		ack->set_accountid(m_accountUID);
		ack->set_accountname(m_name);
		ack->set_conntectedip(ServiceProfile::GetInstance().FindAdvertiseAddress());
		ack->set_connectedport(ServiceProfile::GetInstance().FindAdvertisePort());
		ack->set_isreconnect(isReconnect);
		ack->set_isnewaccount(false);
		ack->set_serverid(ServiceProfile::GetInstance().FindServerID());
		SendToHost(ack);

		m_loginPlatform = loginPlatform;
		m_clientConnectTime = updateTime;

		if (loginIP.has_value())
		{
			m_loginIP = loginIP.value();
		}

		m_requestLoginData = false;
		m_isReconnect = isReconnect;
	}

	void Player::SyncDataFromClient()
	{
		// 동기화가 필요한 정보가 있으면 보내준다.
		SendChangedItem();
		SendChangedEtc();
	}

	void Player::SendChangedItem()
	{
		//const auto noti = NEW(ACChangeItemNoti);
		//SendToHost(noti);
	}

	void Player::SendChangedEtc()
	{
		//const auto noti = NEW(ACChangeEtcNoti);
		//SendToHost(noti);
	}

	void Player::OnDALoginAck(const HostID_t& hostID, const PTR<DALoginAck>& msg)
	{
		if (msg == nullptr) return;

		const auto ack = NEW(ACLoginAck);
		ack->set_accountid(msg->accountid());
		ack->set_accountname(msg->playername());
		ack->set_conntectedip(ServiceProfile::GetInstance().FindAdvertiseAddress());
		ack->set_connectedport(ServiceProfile::GetInstance().FindAdvertisePort());
		ack->set_isreconnect(msg->isreconnect());
		ack->set_isnewaccount(msg->isnewaccount());
		ack->set_serverid(ServiceProfile::GetInstance().FindServerID());
		SendToHost(ack);

		SetPlay(hostID, msg->accountid());

		m_loginPlatform = msg->platformtype();
		m_clientConnectTime = util::TimeUtil::GetInstance().FindKSTDate();

		if (msg->has_ip())
		{
			m_loginIP = msg->ip();
		}
	}

	void Player::OnDAPlayerInfoAck(const HostID_t& hostID, const PTR<DAPlayerInfoAck>& msg)
	{
		if (msg == nullptr) return;
		if (SendNakToHost(hostID, msg->error(), AC_GET_PLAYER_ACK)) return;
		if (m_dbDataLoader == nullptr) return;

		m_dbDataLoader->Receive(msg->messageid());
	}

	void Player::OnDAEndofGameSessionAck(const HostID_t& hostID, const PTR<DAEndofGameSessionAck>& msg)
	{
		if (msg == nullptr) return;
		if (m_dbChannel == nullptr)
		{
			TH_LOG_ERROR(m_hostID, m_accountUID, "dbChannel is null");
			return;
		}
	}

	void Player::OnNetDisconnect(const HostID_t& hostID, const PTR<NetDisconnect>& msg)
	{
		if (msg == nullptr) return;
	}

	void Player::OnCAGetPlayerReq(const HostID_t& hostID, const PTR<CAGetPlayerReq>& msg)
	{
		if (msg == nullptr) return;

		GetPlayerReq();
	}

	LogKey Player::IssueLogKey(const EMessageID& msgID) const
	{
		return LogKeyGenerator::IssueLogKey(msgID);
	}

	PlayerPacketSender::SendToVariable Player::FindSendToVariable() const
	{
		return PlayerPacketSender::SendToVariable
		{
			FindHostID(), FindAccountUID(), m_dbChannel, m_blockDuplicatePacket, FindPID(), true, FindLanguageID()
		};
	}

	const THDateTime& Player::FindPlayerLoginTime() const
	{
		return m_playerLoginTime;
	}

	void Player::ForceDisconnect(const HostID_t& hostID)
	{
		Idle(hostID);
		m_forceDisconnect = true;	// 미션처리를 위하여 바로 끊지 않고, 3초 후에 끊는다. (UpdateEndOfGameSession)
	}
}
