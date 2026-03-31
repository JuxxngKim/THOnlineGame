#pragma once
#include "BaseDispatcher.h"
#include "ConcurrentCenter.h"
#include "SessionPlayer.h"
#include "BlockDuplicatePacket.h"
#include "PlayerPacketSender.h"

namespace th
{
	class PacketDistributor;
	class LoginDataLoader;
	class DuplicateHostManager;

	class Player : public BaseDispatcher, public SessionPlayer, public std::enable_shared_from_this<Player>
	{
		friend class UnexpectedRaidDispatcher;

	private:
		enum class ELastUpdateTimeIndex
		{
			None,
			Frequency,
			ShortTerm,
			LongTerm,
			Max
		};

	private:
		std::string m_pid;
		std::string m_name;
		EPlayerStatus m_status;
		int32_t m_lv;
		int32_t m_exp;
		int32_t m_gameDBID;
		int64_t m_idleTime;
		int32_t m_chattingChannelID;
		THDateTime m_playerLoginTime;
		bool m_reserveNewNReturnPlayerProcess;
		ELanguageCode m_languageID;
		int32_t m_totalPlayTimeMin;

		// XAPlayerDisconnectAck시 초기화
		EPlatformType m_loginPlatform;
		// XAPlayerDisconnectAck시 초기화
		std::string m_loginIP;
		// 클라이언트가 Player를 통해 서버에 붙은 시간
		THDateTime m_clientConnectTime;

		std::unordered_map<ELastUpdateTimeIndex, int64_t> m_lastUpdateTimeMsUnion;

		PTR<Mailbox_t> m_dbChannel;
		UNIQUE_PTR<LoginDataLoader> m_dbDataLoader;
		PTR<BlockDuplicatePacket> m_blockDuplicatePacket;
		PTR<DuplicateHostManager> m_duplicateHostManager;

		std::set<ELoginData> m_syncLoginDatas{};

		int64_t m_loginProcessTime;
		bool m_requestLoginData;
		bool m_isReconnect;
		std::unordered_set<PTR<BaseDispatcher>> m_dispatchers;

		bool m_authenticated;
		int64_t m_loginExpireTimeMS;

		EPlayerDisconnectionStatus m_forceDisconnectionStatus;		// 추방 상태

		bool m_isForceLogin;
		bool m_isFirstLogin;
		bool m_forceDisconnect;

	public:
		Player();
		virtual ~Player();

		Player(const Player& other) = delete;
		Player& operator=(const Player& other) = delete;

		void Clear() override;

	public:
		void RegisterDispatcher();
		void Execute(const PTR<PacketDistributor>& distributor) override;
		void Idle(const HostID_t& hostID) override;
		void SetPlay(const HostID_t& hostID, const AccountUID_t& accountUID);

	public:
		void SetHostID(const HostID_t& hostID) override;
		void SetPID(const std::string& pid);
		const std::string& FindPID() const;
		void SetAccountUID(const AccountUID_t& accountUID) override;
		void SetName(const std::string& name);
		const std::string& FindName() const;
		void SetGameDBID(const int32_t& id);
		int32_t FindGameDBID() const;
		void SetDBChannel(const PTR<Mailbox_t>& channel);
		int32_t FindLevel() const;
		int32_t FindExp() const;
		void SetLevel(const int32_t& level);
		void SetPlayerLoginTime(const MDateTime& loginTime);
		void SetLanguageID(const int32_t& languageID);
		void SetTotalPlayTime(const int32_t& totalPlayTime);
		void SetForceLogin(const bool& isForceLogin);
		int32_t FindTotalPlayTimeMin() const;

		PTR<BlockDuplicatePacket> FindBlockDuplicatePacket();

		ELanguageCode FindLanguageID() const;
		
	public:
		void EndOfGameSession();
		bool IsEndOfGameSession() const;
		bool IsIdle() const;
		bool IsPlaying() const;
		void ChangeSession(const HostID_t& hostID, const bool& isReconnect, const EPlatformType& loginPlatform, const std::optional<std::string>& loginIP);
		void ForceDisconnect(const HostID_t& hostID);

	public:
		const THDateTime& FindPlayerLoginTime() const;

	private:
		template<typename T, typename = std::enable_if_t<std::is_base_of<google::protobuf::Message, T>::value>>
		void SendToHost(const PTR<T>& msg)
		{
			PlayerPacketSender::SendToHost(FindHostID(), msg->messageid(), msg);
		}

		template<typename T, typename Tr, typename = std::enable_if_t<std::is_base_of<google::protobuf::Message, T>::value&& std::is_base_of<google::protobuf::Message, Tr>::value >>
		void SendToDB(const PTR<T>& msg, const PTR<Tr>& requestMsg, bool isInfinite = false)
		{
			PlayerPacketSender::SendToDB(FindSendToVariable(), msg, requestMsg, isInfinite);
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of<google::protobuf::Message, T>::value>>
		void SendToDB(const PTR<T>& msg, const EMessageID& reqMsgID, bool isInfinite = false)
		{
			PlayerPacketSender::SendToDB(FindSendToVariable(), msg, reqMsgID, isInfinite);
		}

	public:
		bool SendNakToHost(const HostID_t& hostID, const EErrorMsg& err, const EMessageID& successfulMsgID)
		{
			if (err == E_SUCCESS) return false;

			const auto nak = NEW(ACCommonNak);
			nak->set_error(err);
			nak->set_successfulmsgid(successfulMsgID);
			SendToHost(nak);

			// 해당 에러가 발생하는 경우에는 강제 종료 처리를 한다.
			if (err == E_WRONG_PLAYER_DATA_CLIENT_RECONNECT
				|| err == E_DB_DUPLICATE_CONNECTION
				|| err == E_INTERNAL_DATABASE_ERROR)
			{
				ForceDisconnect(hostID);
			}

			return true;
		}

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<google::protobuf::Message, T>>>
		bool SendToSelf(const PTR<T>& msg)
		{
			if (msg == nullptr) return false;

			ConcurrentCenter::GetInstance().SendToLogicService(NEW(PacketWrapper, FindHostID(), FindAccountUID(), msg->messageid(), msg));
			return true;
		}

	private:
		LogKey IssueLogKey(const EMessageID& msgID) const;
		void SyncDataFromClient();
		void SendChangedItem();
		void SendChangedEtc();

		void Update();
		void FrequencyUpdate(const int64_t& curTimeMs);
		void ShortTermUpdate(const int64_t& curTimeMs);
		void LongTermUpdate(const int64_t& curTimeMs);
		void UpdateEndOfGameSession();

		void SyncPlayerDataToClient();
		void GetPlayerReq();

		PlayerPacketSender::SendToVariable FindSendToVariable() const;

	public:
		void OnNetDisconnect(const HostID_t& hostID, const PTR<NetDisconnect>& msg);
		void OnCAGetPlayerReq(const HostID_t& hostID, const PTR<CAGetPlayerReq>& msg);

		void OnDALoginAck(const HostID_t& hostID, const PTR<DALoginAck>& msg);
		void OnDAPlayerInfoAck(const HostID_t& hostID, const PTR<DAPlayerInfoAck>& msg);
		void OnDAEndofGameSessionAck(const HostID_t& hostID, const PTR<DAEndofGameSessionAck>& msg);
	};
}