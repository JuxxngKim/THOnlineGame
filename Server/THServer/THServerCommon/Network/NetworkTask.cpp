#include "CommonPch.h"
#include "NetworkTask.h"
#include "NetworkManager.h"

namespace network
{
	////////////////////////////////////////////////////////////////////
	void NetworkTask::Failed()
	{
		auto ncc = NEW(NetworkEventTask);
		ncc->m_success = false;
		ncc->m_target = this;
		NetworkManager::GetInstance().ControllerPush(ncc);
	}

	////////////////////////////////////////////////////////////////////
	ENetworkTask NetworkTaskConnect::GetType()
	{
		return ENetworkTask::Connect;
	}

	void NetworkTaskConnect::Prepare()
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		ZeroMemory((LPOVERLAPPED)this, sizeof(OVERLAPPED));

		//소켓생성
		SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (sock == INVALID_SOCKET)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "failed - WSASocket : [err:%]", WSAGetLastError());
			Failed();
			return;
		}

		if (host->SetSocket(sock) == false)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "SetSocket Failed");
			Failed();
			return;
		}

		//포트할당
		SOCKADDR_IN local = {};
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		if (bind(host->GetSocket(), (SOCKADDR*)&local, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "failed - bind : [err:%]", WSAGetLastError());
			Failed();
			return;
		}

		//주소변환
		auto ip = host->GetIP();
		auto port = host->GetPort();

		SOCKADDR_IN remote = {};
		remote.sin_family = AF_INET;
		remote.sin_port = htons((unsigned short)port);

		if (!ip.empty())
		{
			if (isalpha(ip[0]))
			{
				addrinfo* infos = nullptr;
				addrinfo hints = {};
				hints.ai_family = remote.sin_family;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;

				getaddrinfo(ip.c_str(), nullptr, &hints, &infos);
				if (infos)
				{
					memcpy(&remote, infos->ai_addr, infos->ai_addrlen);
					freeaddrinfo(infos);
				}
				else
				{
					inet_pton(remote.sin_family, "127.0.0.1", &remote.sin_addr);
				}
			}
			else
			{
				inet_pton(remote.sin_family, ip.c_str(), &remote.sin_addr);
			}
		}
		else
		{
			inet_pton(remote.sin_family, "127.0.0.1", &remote.sin_addr);
		}

		//connect 요청
		static LPFN_CONNECTEX ConnectEx = nullptr;
		if (ConnectEx == nullptr)
		{
			GUID guid = WSAID_CONNECTEX;
			DWORD bytes = 0;
			if (WSAIoctl(host->GetSocket(), SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &ConnectEx, sizeof(ConnectEx), &bytes, nullptr, nullptr) != 0)
			{
				TH_LOG_ERROR(host->GetHostID(), 0, "WSAIoctl failed : [err:%]", WSAGetLastError());
				Failed();
				return;
			}
		}

		if (ConnectEx(host->GetSocket(), (sockaddr*)&remote, sizeof(remote), nullptr, 0, nullptr, (LPOVERLAPPED)this) == FALSE)
		{
			int32_t error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				TH_LOG_ERROR(host->GetHostID(), 0, "ConnectEx failed : [err:%]", error);
				Failed();
				return;
			}
		}
	}

	void NetworkTaskConnect::Complete(bool result, int32_t transferred)
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//요청결과 체크
		if (result == false)
		{
			TH_LOG_CRITICAL(host->GetHostID(), 0, "result failed");
			Failed();
			return;
		}

		//시작 처리
		auto ncc = NEW(NetworkEventTask);
		ncc->m_success = true;
		ncc->m_target = this;
		NetworkManager::GetInstance().ControllerPush(ncc);
	}

	////////////////////////////////////////////////////////////////////
	ENetworkTask NetworkTaskAccept::GetType()
	{
		return ENetworkTask::Accept;
	}

	void NetworkTaskAccept::Prepare()
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//task 초기화
		ZeroMemory((LPOVERLAPPED)this, sizeof(OVERLAPPED));
		ZeroMemory(m_address, sizeof(m_address));

		//소켓생성
		m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (m_socket == INVALID_SOCKET)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "WSASocket failed : [err:%]", WSAGetLastError());
			Failed();
			return;
		}

		//accept 요청
		DWORD bytes = 0;

		if (AcceptEx(host->GetSocket(), m_socket, m_address, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPOVERLAPPED)this) == FALSE)
		{
			int32_t error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				TH_LOG_ERROR(host->GetHostID(), 0, "AcceptEx failed : [err:%]", error);
				Failed();
				return;
			}
		}
	}

	void NetworkTaskAccept::Complete(bool result, int32_t transferred)
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//요청결과 체크
		if (result == true)
		{
			//SO_UPDATE_ACCEPT_CONTEXT
			SOCKET listener = host->GetSocket();
			setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listener, sizeof(listener));

			//접속주소 얻기
			sockaddr_in* local = nullptr;
			int32_t localLength = 0;
			sockaddr_in* remote = nullptr;
			int32_t remoteLength = 0;
			GetAcceptExSockaddrs((PVOID)m_address, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (sockaddr**)&local, &localLength, (sockaddr**)&remote, &remoteLength);

			if (remote)
			{
				auto ncc = NEW(NetworkEventAccept);
				ncc->m_callback = host->GetCallback();
				ncc->m_ip = inet_ntoa(remote->sin_addr);
				ncc->m_port = (int32_t)ntohs(remote->sin_port);
				ncc->m_timeoutMs = host->GetTimeoutMs();
				ncc->m_socket = m_socket;
				NetworkManager::GetInstance().ControllerPush(ncc);
			}
		}
		else
		{
			closesocket(m_socket);
		}

		m_socket = INVALID_SOCKET;

		//accept 요청
		Prepare();
	}

	////////////////////////////////////////////////////////////////////
	ENetworkTask NetworkTaskReceive::GetType()
	{
		return ENetworkTask::Receive;
	}

	void NetworkTaskReceive::Prepare()
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//OVERLAPPED 초기화
		ZeroMemory((LPOVERLAPPED)this, sizeof(OVERLAPPED));

		//receive 요청
		WSABUF wsabuf = {};
		DWORD bytes = 0;
		DWORD flag = 0;

		if (m_buffer.ReadyToRecv(wsabuf) == false)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "ReadyToRecv failed");
			Failed();
			return;
		}

		if (WSARecv(host->GetSocket(), &wsabuf, 1, &bytes, &flag, (LPOVERLAPPED)this, nullptr) == SOCKET_ERROR)
		{
			int32_t error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				TH_LOG_ERROR(host->GetHostID(), 0, "failed - WSARecv:%", error);
				Failed();
				return;
			}
		}
	}

	void NetworkTaskReceive::Complete(bool result, int32_t transferred)
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//요청결과 체크
		if (result == false || transferred <= 0)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "failed - result");
			Failed();
			return;
		}

		//받은 사이즈 추가
		m_buffer.CompleteWrite(transferred);

		while (true)
		{
			const auto& [messageID, messageBuffer, messageSize] = m_buffer.ByteToPacket(host->IsEncrypted(), host->FindNonce());
			if (messageID <= 0) break;

			host->PacketCallback(messageID, messageBuffer, messageSize);
		}

		//receive 요청
		Prepare();
	}

	////////////////////////////////////////////////////////////////////
	ENetworkTask NetworkTaskSend::GetType()
	{
		return ENetworkTask::Send;
	}

	void NetworkTaskSend::Prepare()
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//OVERLAPPED 초기화
		ZeroMemory((LPOVERLAPPED)this, sizeof(OVERLAPPED));

		m_buffers.resize(m_datas.size());
		m_total = 0;

		int32_t cnt = 0;
		for (auto& data : m_datas)
		{
			//데이터 복사
			auto& wsabuf = m_buffers[cnt];
			wsabuf.buf = data->GetData();
			data->Encrypt(host->IsEncrypted(), host->FindNonce());
			wsabuf.len = data->GetDataSize();
			m_total += wsabuf.len;
			++cnt;
		}

		//send 요청
		DWORD bytes = 0;
		DWORD flag = 0;
		auto a = &m_buffers.front();
		if (WSASend(host->GetSocket(), m_buffers.data(), (DWORD)m_buffers.size(), &bytes, flag, (LPOVERLAPPED)this, nullptr) == SOCKET_ERROR)
		{
			int32_t error = WSAGetLastError();
			if (error != WSA_IO_PENDING)
			{
				TH_LOG_ERROR(host->GetHostID(), 0, "failed - WSASend:%", error);
				Failed();
				return;
			}
		}
	}

	void NetworkTaskSend::Complete(bool result, int32_t transferred)
	{
		auto host = (NetworkHost*)m_owner;
		if (host == nullptr)
		{
			//이게 발생하면..애초에 구현을 잘못한것..로그만 남긴다
			TH_LOG_ERROR(0, 0, "host null");
			Failed();
			return;
		}

		//요청결과 체크
		if (result == false || transferred <= 0)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "failed - result");
			Failed();
			return;
		}

		//다 보냈는지 체크
		if (transferred != m_total)
		{
			TH_LOG_ERROR(host->GetHostID(), 0, "transferred:%/%", transferred, m_total);
			return;
		}

		//send 완료 처리
		auto ncc = NEW(NetworkEventTask);
		ncc->m_success = true;
		ncc->m_target = this;
		NetworkManager::GetInstance().ControllerPush(ncc);
	}
}