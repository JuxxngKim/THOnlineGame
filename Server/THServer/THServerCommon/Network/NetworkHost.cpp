#include "CommonPch.h"
#include "NetworkHost.h"
#include "NetworkManager.h"
#include "TimeUtil.h"
//#include "MessageCreator.h"
#include "DefineConst.h"
#include "PacketWrapper.h"

namespace network
{
	NetworkHost::NetworkHost(const HostID_t& hostID)
		: m_hostId(hostID)
		, m_timeoutMs{ NETWORK_TIMEOUT_MS }
		, m_encrypted{ false }
		, m_nonceAssigned{ false }
	{
		//th::CommonMetric::GetInstance().IncrementObject("network_host");
	}

	NetworkHost::~NetworkHost()
	{
		//th::CommonMetric::GetInstance().DecrementObject("network_host");
	}

	ENetworkHost NetworkHost::GetHostType() const
	{
		return m_hostType;
	}

	HostID_t NetworkHost::GetHostID() const
	{
		return m_hostId;
	}

	SOCKET NetworkHost::GetSocket() const
	{
		return m_socket;
	}

	std::string NetworkHost::GetIP()
	{
		return m_ip;
	}

	int32_t NetworkHost::GetPort() const
	{
		return m_port;
	}

	int32_t NetworkHost::GetTimeoutMs() const
	{
		return m_timeoutMs;
	}

	bool NetworkHost::SetSocket(SOCKET sock)
	{
		if (sock == INVALID_SOCKET) return false;

		m_socket = sock;

		int32_t flag = 1;
		setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

		//iocp와 연결
		if (NetworkManager::GetInstance().WorkerBind(m_socket) == false)
		{
			TH_LOG_ERROR(m_hostId, 0, "failed - Bind");
			return false;
		}

		return true;
	}

	void NetworkHost::SetHostValue(const int64_t value)
	{
		m_hostValue = value;
	}

	NetworkCallback NetworkHost::GetCallback()
	{
		return m_callback;
	}

	void NetworkHost::ExecuteCallback(int32_t messageID, const PTR<google::protobuf::Message>& message)
	{
		if (m_callback == nullptr)
		{
			TH_LOG_ERROR(m_hostId, 0, "failed - m_callback");
			Close();
			return;
		}

		auto interMsg = NEW(th::PacketWrapper, m_hostId, m_hostValue, messageID, message);
		m_callback(interMsg);
	}

	void NetworkHost::PacketCallback(int32_t messageID, char* messageBuffer, int32_t messageSize)
	{
		//받기 완료 처리
		switch (messageID)
		{
		case th::NET_CONNECT:
		{
			auto msg = NEW(th::NetConnect);
			msg->ParseFromArray(messageBuffer, messageSize);

			//시간설정
			m_timeoutMs = msg->timeoutms();

			int64_t currentTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs();
			m_checkTimeMs = currentTimeMs + m_timeoutMs;

			ExecuteCallback(messageID, msg);
			break;
		}
		case th::NET_ALIVE_REQ:
		{
			//시간설정
			int64_t currentTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs();
			m_checkTimeMs = currentTimeMs + m_timeoutMs;

			auto req = NEW(th::NetAliveReq);
			req->ParseFromArray(messageBuffer, messageSize);

			//ExecuteCallback(messageID, req);

			//alive
			auto ack = NEW(th::NetAliveAck);
			ack->set_requestms(req->requestms());
			ack->set_responsems(util::TimeUtil::GetInstance().FindUTCMs());

			NetworkManager::GetInstance().Send(m_hostId, ack->messageid(), ack);

			break;
		}
		case th::NET_ALIVE_ACK:
		{
			//시간설정
			int64_t currentTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs();
			m_checkTimeMs = currentTimeMs + m_timeoutMs;
			break;
		}
		default:
		{
			/*auto msg = th::MessageCreator::GetInstance().CreateMessage(messageID);
			if (msg == nullptr)
			{
				TH_LOG_ERROR(m_hostId, 0, "CreateMessage : [id:%, name:%]", messageID, th::EMessageID_Name((th::EMessageID)messageID));
				return;
			}

			msg->ParseFromArray(messageBuffer, messageSize);

			ExecuteCallback(messageID, msg);*/
			break;
		}
		}

		//th::CommonMetric::GetInstance().IncrementPacketReceiveCounter(messageID);
	}

	//process
	bool NetworkHost::Update(int64_t currentTimeMs)
	{
		//host 종료 체크
		if (m_socket == INVALID_SOCKET && m_tasks.empty() == true)
		{
			return false;
		}

		//timeout 체크
		if (m_hostType == ENetworkHost::Connecter || m_hostType == ENetworkHost::Accepter)
		{
			if (currentTimeMs > m_checkTimeMs)
			{
				Close();
				return true;
			}
		}

		//Connecter 타입은 NetAliveReq 패킷 보낼지 체크
		if (m_hostType == ENetworkHost::Connecter)
		{
			if (currentTimeMs > m_aliveTimeMs)
			{
				m_aliveTimeMs = currentTimeMs + NETWORK_ALIVE_MS;

				//alive
				auto msg = NEW(th::NetAliveReq);
				msg->set_requestms(util::TimeUtil::GetInstance().FindUTCMs());
				NetworkManager::GetInstance().Send(m_hostId, msg->messageid(), msg);
			}
		}

		return true;
	}

	void NetworkHost::Refresh()
	{
		int64_t currentTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs();
		m_checkTimeMs = currentTimeMs + m_timeoutMs;
		m_aliveTimeMs = currentTimeMs + NETWORK_ALIVE_MS;
	}

	void NetworkHost::RemoveTask(NetworkTask* task)
	{
		if (task == nullptr) return;

		for (auto iter = m_tasks.begin(); iter != m_tasks.end(); ++iter)
		{
			if (iter->get() == task)
			{
				m_tasks.erase(iter);
				break;
			}
		}
	}

	bool NetworkHost::IsPossibleSend()
	{
		for (auto& task : m_tasks)
		{
			if (task->GetType() == ENetworkTask::Send) return false;
		}

		return true;
	}

	void NetworkHost::Connect(NetworkCallback callback, std::string ip, int32_t port)
	{
		m_callback = callback;
		m_ip = ip;
		m_port = port;

		auto task = NEW(NetworkTaskConnect);
		task->m_owner = this;

		if (NetworkManager::GetInstance().WorkerPush(task.get()))
		{
			m_tasks.emplace_back(task);
		}
	}

	void NetworkHost::Connected()
	{
		m_hostType = ENetworkHost::Connecter;

		Refresh();

		//receive 요청
		auto task = NEW(NetworkTaskReceive);
		task->m_owner = this;

		if (NetworkManager::GetInstance().WorkerPush(task.get()))
		{
			m_tasks.emplace_back(task);
		}
	}

	void NetworkHost::Listen(NetworkCallback callback, std::string ip, int32_t port, int32_t timeoutMs)
	{
		m_hostType = ENetworkHost::Listener;
		m_callback = callback;
		m_ip = ip;
		m_port = port;
		m_timeoutMs = timeoutMs;

		Refresh();

		//소켓생성
		SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (sock == INVALID_SOCKET)
		{
			TH_LOG_ERROR(m_hostId, 0, "failed - WSASocket : [err:%]", WSAGetLastError());
			return;
		}

		SetSocket(sock);

		//포트할당
		SOCKADDR_IN local = {};
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		local.sin_port = htons((unsigned short)m_port);
		if (bind(sock, (SOCKADDR*)&local, sizeof(local)) == SOCKET_ERROR)
		{
			TH_LOG_ERROR(m_hostId, 0, "failed - bind : [err:%]", WSAGetLastError());
			return;
		}

		//대기
		if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
		{
			TH_LOG_ERROR(m_hostId, 0, "failed - listen : [err:%]", WSAGetLastError());
			return;
		}

		// TODO
		//// 서버시작 이벤트 발생
		//auto msg = NEW(th::InternalEventTick);
		//msg->set_eventtype(static_cast<int32_t>(th::EEventType::ReadyServer));
		//ExecuteCallback(msg->messageid(), msg);

		//accept 요청
		for (int32_t n = 0; n < MAX_ACCEPT_COUNT; ++n)
		{
			auto task = NEW(NetworkTaskAccept);
			task->m_owner = this;

			if (NetworkManager::GetInstance().WorkerPush(task.get()))
			{
				m_tasks.emplace_back(task);
			}
		}
	}

	void NetworkHost::Accept(NetworkCallback callback, std::string ip, int32_t port, int32_t timeoutMs, SOCKET sock)
	{
		m_hostType = ENetworkHost::Accepter;
		m_callback = callback;
		m_ip = ip;
		m_port = port;
		m_timeoutMs = timeoutMs;

		Refresh();

		//소켓셋팅
		SetSocket(sock);

		//접속메세지 보냄
		auto msg = NEW(th::NetConnect);
		msg->set_connectip(m_ip);
		msg->set_connectport(m_port);
		msg->set_timeoutms(m_timeoutMs);
		msg->set_servertime(util::TimeUtil::GetInstance().FindUTCMs());

		//내부로 접속한 유저가 있다고 알려준다.
		ExecuteCallback(msg->messageid(), msg);

		//접속한사람한테 알려준다.
		NetworkManager::GetInstance().Send(m_hostId, msg->messageid(), msg);

		//receive 요청
		auto task = NEW(NetworkTaskReceive);
		task->m_owner = this;

		if (NetworkManager::GetInstance().WorkerPush(task.get()))
		{
			m_tasks.emplace_back(task);
		}
	}

	void NetworkHost::Send(PTR<NetworkBuffer> buffer /*= nullptr*/)
	{
		if (m_hostType == ENetworkHost::Listener) return;
		if (m_socket == INVALID_SOCKET) return;

		if (buffer)
		{
			m_datas.emplace_back(buffer);
		}

		if (m_datas.size() > 0)
		{
			if (IsPossibleSend() == true)
			{
				//send 요청
				auto task = NEW(NetworkTaskSend);
				task->m_owner = this;
				task->m_datas.swap(m_datas);

				if (NetworkManager::GetInstance().WorkerPush(task.get()))
				{
					m_tasks.emplace_back(task);
				}
			}
		}
	}

	void NetworkHost::Close()
	{
		if (m_socket != INVALID_SOCKET)
		{
			LINGER opt{};
			opt.l_onoff = 1;
			opt.l_linger = 0;
			setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt));

			closesocket(m_socket);
			m_socket = INVALID_SOCKET;

			m_datas.clear();
		}
	}

	bool NetworkHost::IsEncrypted() const
	{
		return m_encrypted;
	}

	void NetworkHost::SetEncrypted()
	{
		m_encrypted = true;
	}

	void NetworkHost::SetNonce(unsigned char nonce[])
	{
		if (!IsEncrypted() || m_nonceAssigned) return;
		//memcpy_s(m_nonce, crypto_stream_chacha20_NONCEBYTES, nonce, crypto_stream_chacha20_NONCEBYTES);
		m_nonceAssigned = true;
	}

	const unsigned char* NetworkHost::FindNonce() const
	{
		return nullptr;
		//return m_nonce;
	}
}
