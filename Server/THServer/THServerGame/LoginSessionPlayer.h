#pragma once
#include "SessionPlayer.h"
#include "BaseDispatcher.h"
#include "ConcurrentCenter.h"

namespace th
{
	class LoginSessionPlayer : public std::enable_shared_from_this<LoginSessionPlayer>, public SessionPlayer, public BaseDispatcher
	{

	private:
		const int32_t RetryCheckCount = 5;
		const int64_t RetryCheckMs = 500;

	private:
		std::string m_pid;

	public:
		LoginSessionPlayer() = delete;
		LoginSessionPlayer(const HostID_t& hostID, const std::string& pid);
		virtual ~LoginSessionPlayer();

		void Execute(const PTR<PacketDistributor>& distributor) override;
		void Idle(const HostID_t& hostID) override {}

		void Clear() override;

	private:
		void SendToDB(const HostID_t& hostID, int32_t messageID, const PTR<google::protobuf::Message>& msg)
		{
			auto innerMsg = NEW(PacketWrapper, hostID, 0, messageID, msg);
			ConcurrentCenter::GetInstance().SendToSystemDB(hostID, innerMsg);
		}

		void SendTo(int32_t messageID, const PTR<google::protobuf::Message>& message)
		{
			network::NetworkManager::GetInstance().Send(m_hostID, messageID, message);
		}

	private:
		void OnNetConnect(const HostID_t& hostID, const PTR<NetConnect>& msg) {}
		void OnNetDisconnect(const HostID_t& hostID, const PTR<NetDisconnect>& msg) {}
		void OnCALoginReq(const HostID_t& hostID, const PTR<CALoginReq>& msg);
		void OnACLoginAck(const HostID_t& hostID, const PTR<ACLoginAck>& msg);
		void OnDALoginNak(const HostID_t& hostID, const PTR<DALoginNak>& msg);
	};
}