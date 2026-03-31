#pragma once
#include "LogicEventor.h"
#include "sprotocol.pb.h"

namespace th
{
	struct PacketWrapper;
	class Player;
	class PlayerArchive;
	class LoginPendingList;
	class WaitingSession;
	class OutGameLogicEventor : public LogicEventor
	{
	private:
		int64_t m_lastServerInfoSyncTime;
		int64_t m_lastBiCurrentUserSyncTime;
		int64_t m_lastWaitingMessageInfoTime;
		int64_t m_nextPlayerCountSyncTime;
		int64_t m_nextServerAliveSyncTime;

		UNIQUE_PTR<PlayerArchive> m_playerArchive;
		UNIQUE_PTR<LoginPendingList> m_loggingInArchive;
		UNIQUE_PTR<WaitingSession> m_waitingSessions;

		std::unordered_set<EMessageID> m_waitingMessageID;
		google::protobuf::RepeatedPtrField<std::string> m_loggedInPIDs;

	public:
		OutGameLogicEventor() = delete;
		OutGameLogicEventor(const PTR<PacketCollector<PTR<PacketWrapper>>>& collector);
		virtual ~OutGameLogicEventor();

		void Event(const int64_t& delta) override;
		void UpdateServerInfoRedis(const int64_t& delta);
		void UpdatePlayerCount(const int64_t& delta);
		void SyncAlive(const int64_t& delta);


	private:
		// process message
		void OnNetDisconnect(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<NetDisconnect>& msg, const uint8_t& flag);

		void OnCALoginReq(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<CALoginReq>& msg, const uint8_t& flag);
		void OnCAGetPlayerReq(const HostID_t& hostID, const AccountUID_t& accountUID, const PTR<CAGetPlayerReq>& msg, const uint8_t& flag);

		void OnDALoginAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DALoginAck>& msg, const uint8_t& flag);
		void OnDAEndofGameSessionAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DAEndofGameSessionAck>& msg, const uint8_t& flag);

	private:
		void PrepareNetDisconnect(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<NetDisconnect>& msg);
		void ArrangeNetDisconnect(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<NetDisconnect>& msg);
		void PrepareLoginAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DALoginAck>& msg);
		void ArrangeLoginAck(const HostID_t& hostID, const AccountUID_t& accountID, const PTR<DALoginAck>& msg);

		void UpdateServerInfo();
		void UpdateBICurrentUser();
	};
}
