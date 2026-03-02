#include "CommonPch.h"
#include "NetworkEvent.h"
#include "NetworkManager.h"

namespace network
{
	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventTask::GetType()
	{
		return ENetworkEvent::Task;
	}

	void NetworkEventTask::Run()
	{
		auto task = (NetworkTask*)m_target;
		if (task == nullptr)
		{
			TH_LOG_ERROR(0, 0, "task null");
			return;
		}

		auto type = task->GetType();
		auto host = (NetworkHost*)task->m_owner;
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "host null");
			return;
		}

		host->RemoveTask(task);

		if (m_success)
		{
			switch (type)
			{
			case ENetworkTask::Connect:
				host->Connected();
				break;

			case ENetworkTask::Send:
				host->Send();
				break;

			default:
				TH_LOG_ERROR(host->GetHostID(), 0, "unknown task : [type:%]", static_cast<int32_t>(type));
				host->Close();
				break;
			}
		}
		else
		{
			host->Close();
		}
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventConnect::GetType()
	{
		return ENetworkEvent::Connect;
	}

	void NetworkEventConnect::Run()
	{
		//host 생성
		auto host = NetworkManager::GetInstance().CreateHost();
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "CreateHost failed");
			return;
		}

		host->Connect(m_callback, m_ip, m_port);
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventListen::GetType()
	{
		return ENetworkEvent::Listen;
	}

	void NetworkEventListen::Run()
	{
		//host 생성
		auto host = NetworkManager::GetInstance().CreateHost();
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "CreateHost failed");
			return;
		}

		host->Listen(m_callback, m_ip, m_port, m_timeoutMs);
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventAccept::GetType()
	{
		return ENetworkEvent::Accept;
	}

	void NetworkEventAccept::Run()
	{
		//host 생성
		auto host = NetworkManager::GetInstance().CreateHost();
		if (host == nullptr)
		{
			TH_LOG_ERROR(0, 0, "CreateHost failed");
			return;
		}

		host->Accept(m_callback, m_ip, m_port, m_timeoutMs, m_socket);
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventSend::GetType()
	{
		return ENetworkEvent::Send;
	}

	void NetworkEventSend::Run()
	{
		//host 체크
		for (auto hostID : m_hostIds)
		{
			auto host = NetworkManager::GetInstance().FindHost(hostID);
			if (host == nullptr) continue;

			//리스트에 추가
			if (1 < m_hostIds.size())
			{
				const auto& cpyBuffer = NEW(NetworkBuffer, m_buffer);
				host->Send(cpyBuffer);
			}
			else
			{
				host->Send(m_buffer);
			}

			if (!m_json.empty())
			{
				//SK2_LOG_TRACEMESSAGE(hostID, 0, "%", evt->m_json);
			}
		}
	}

	void NetworkEventSend::Serialize()
	{
		NetworkManager::GetInstance().TraceMessage(m_json, m_packetId, *m_packet);

		m_buffer = NEW(NetworkBuffer, m_ignoreEncrypt);
		if (m_buffer->PacketToByte(m_packetId, *m_packet) == false)
		{
			TH_LOG_ERROR(0, 0, "failed PacketToByte : [id:%]", m_packetId);
		}

		m_step = ESerialize::Completed;
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventClose::GetType()
	{
		return ENetworkEvent::Close;
	}

	void NetworkEventClose::Run()
	{
		for (auto hostID : m_hostIds)
		{
			auto host = NetworkManager::GetInstance().FindHost(hostID);
			if (host == nullptr) continue;
			if (host->GetHostType() == ENetworkHost::Listener) continue;

			//host 해제
			host->Close();
		}
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventRegisterValue::GetType()
	{
		return ENetworkEvent::RegisterValue;
	}

	void NetworkEventRegisterValue::Run()
	{
		auto host = NetworkManager::GetInstance().FindHost(m_hostId);
		if (host == nullptr) return;

		host->SetHostValue(m_value);
	}

	////////////////////////////////////////////////////////////////////
	ENetworkEvent NetworkEventEncrypt::GetType()
	{
		return ENetworkEvent::Encrypt;
	}

	void NetworkEventEncrypt::Run()
	{
		const auto& host = NetworkManager::GetInstance().FindHost(m_hostID);
		if (host == nullptr) return;

		host->SetEncrypted();
	}

	ENetworkEvent NetworkEventNonce::GetType()
	{
		return ENetworkEvent::Nonce;
	}

	void NetworkEventNonce::Run()
	{
		const auto& host = NetworkManager::GetInstance().FindHost(m_hostID);
		if (host == nullptr) return;

		//host->SetNonce(m_nonce);
	}
}
