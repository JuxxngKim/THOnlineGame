#pragma once

//#include "CommonMetric.h"
#include "NetworkContext.h"
#include "NetworkBuffer.h"

namespace network
{
	struct NetworkEvent
	{
		virtual ENetworkEvent GetType() = 0;
		virtual void Run() = 0;
	};

	struct NetworkEventTask : NetworkEvent
	{
		bool m_success{ false };
		void* m_target{ nullptr };

		NetworkEventTask()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_task");
		}
		~NetworkEventTask()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_task");
		}

		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventConnect : NetworkEvent
	{
		NetworkCallback m_callback{ nullptr };
		std::string m_ip{};
		int32_t m_port{ 0 };

		NetworkEventConnect()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_connect");
		}
		~NetworkEventConnect()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_connect");
		}
		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventListen : NetworkEvent
	{
		NetworkCallback m_callback{ nullptr };
		std::string m_ip{};
		int32_t m_port{ 0 };
		int32_t m_timeoutMs{ 0 };

		NetworkEventListen()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_listen");
		}
		~NetworkEventListen()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_listen");
		}
		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventAccept : NetworkEvent
	{
		NetworkCallback m_callback{ nullptr };
		std::string m_ip{};
		int32_t m_port{ 0 };
		int32_t m_timeoutMs{ 0 };
		SOCKET m_socket{ INVALID_SOCKET };

		NetworkEventAccept()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_accept");
		}
		~NetworkEventAccept()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_accept");
		}
		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventSend : NetworkEvent
	{
		std::unordered_set<HostID_t> m_hostIds{};
		int32_t m_packetId{ 0 };
		PTR<google::protobuf::Message> m_packet{ nullptr };
		PTR<NetworkBuffer> m_buffer{ nullptr };
		std::string m_json{};
		bool m_ignoreEncrypt{ false };
		volatile ESerialize m_step{ ESerialize::None };

		NetworkEventSend()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_send");
		}
		~NetworkEventSend()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_send");
		}
		virtual ENetworkEvent GetType() override;
		virtual void Run() override;

		void Serialize();
	};

	struct NetworkEventClose : NetworkEvent
	{
		std::unordered_set<HostID_t> m_hostIds{};

		NetworkEventClose()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_close");
		}
		~NetworkEventClose()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_close");
		}
		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventRegisterValue : NetworkEvent
	{
		HostID_t m_hostId{ 0 };
		int64_t m_value{ 0 };

		NetworkEventRegisterValue()
		{
			//::CommonMetric::GetInstance()->IncrementObject("network_event_register");
		}
		~NetworkEventRegisterValue()
		{
			//::CommonMetric::GetInstance()->DecrementObject("network_event_register");
		}
		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventEncrypt : NetworkEvent
	{
		HostID_t m_hostID{ 0 };

		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};

	struct NetworkEventNonce : NetworkEvent
	{
		HostID_t m_hostID{ 0 };
		//unsigned char m_nonce[crypto_stream_chacha20_NONCEBYTES];

		virtual ENetworkEvent GetType() override;
		virtual void Run() override;
	};
}
