#include "GamePch.h"
#include "LoginSessionPlayer.h"
#include "PacketDistributor.h"
#include "ProtoMemberAdapter.h"
#include "TimeUtil.h"

namespace th
{
	LoginSessionPlayer::LoginSessionPlayer(const HostID_t& hostID, const std::string& pid) : SessionPlayer(hostID), m_pid{ pid }
	{
		RegisterHandler(this, &LoginSessionPlayer::OnNetConnect);
		RegisterHandler(this, &LoginSessionPlayer::OnNetDisconnect);
		RegisterHandler(this, &LoginSessionPlayer::OnCALoginReq);
		RegisterHandler(this, &LoginSessionPlayer::OnACLoginAck);
		RegisterHandler(this, &LoginSessionPlayer::OnDALoginNak);
	}

	LoginSessionPlayer::~LoginSessionPlayer()
	{
	}

	void LoginSessionPlayer::Execute(const PTR<PacketDistributor>& distributor)
	{
		if (distributor == nullptr) return;

		const auto& queue = distributor->Pick(m_hostID);
		for (const auto& packet : queue)
		{
			Dispatch(packet->HostID, packet->MessageID, packet->Msg);
		}
	}

	void LoginSessionPlayer::Clear()
	{
		SessionPlayer::Clear();
		m_pid = "";
	}

	void LoginSessionPlayer::OnCALoginReq(const HostID_t& hostID, const PTR<CALoginReq>& msg)
	{
		if (msg == nullptr) return;

		auto Nak = [&](const EErrorMsg err)
			{
				if (err == E_MAINTENANCE_SERVER)
				{
					TH_LOG_ERROR(m_hostID, 0, "Is maintenance Error.");
					return;
				}

				const auto nak = NEW(ACCommonNak);
				nak->set_error(err);
				nak->set_successfulmsgid(AC_LOGIN_ACK);
				network::NetworkManager::GetInstance().Send(m_hostID, nak->messageid(), nak);
			};

		if (msg->currentversion() != EProtocolVersion::RE_PROTOCOL_VERSION)
		{
			return Nak(E_INVALID_PROTOCOL_VERSION);
		}

		const auto req = NEW(ADLoginReq);
		req->set_pid(msg->pid());
		req->set_logkey(util::TimeUtil::GetInstance().FindKSTMs());
		req->set_isreconnect(msg->isreconnect());
		req->set_serverid(ServiceProfile::GetInstance().FindServerID());
		if (msg->has_appversion())
		{
			req->set_appversion(msg->appversion());
		}
		else
		{
			req->set_appversion("0.0.0.0");
		}

		ProtoMemberAdapter::ConvertReDateTimeToMDateTime(util::TimeUtil::GetInstance().FindKSTDate(), req->mutable_updatedate());
		req->set_languageid(msg->languageid());
		req->set_platformtype(msg->platformtype());
		req->set_ip(msg->ip());
		SendToDB(hostID, req->messageid(), req);
	}

	void LoginSessionPlayer::OnACLoginAck(const HostID_t& hostID, const PTR<ACLoginAck>& msg)
	{
		//NOTE::접속 대기중인 상태. 해당 서버에 접속중인 동일한 계정이 없으면 들어오지 않는다.
		m_accountUID = msg->accountid();
	}

	void LoginSessionPlayer::OnDALoginNak(const HostID_t& hostID, const PTR<DALoginNak>& msg)
	{
		if (msg == nullptr) return;
		if (msg->isforcelogin()) return;

		const auto nak = NEW(ACCommonNak);
		nak->set_error(msg->error());
		nak->set_successfulmsgid(AC_LOGIN_ACK);
		network::NetworkManager::GetInstance().Send(m_hostID, nak->messageid(), nak);
	}
}