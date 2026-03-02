#pragma once
//#include <sodium.h>
#include "NetworkTask.h"

namespace network
{
	class NetworkHost
	{
	private:
		ENetworkHost m_hostType{ ENetworkHost::None };
		HostID_t m_hostId{ 0 };
		int64_t m_hostValue{ 0 };

		NetworkCallback m_callback{};
		std::string m_ip{};
		int32_t m_port{ 0 };
		SOCKET m_socket{ INVALID_SOCKET };

		std::list<PTR<NetworkTask>> m_tasks;

		std::deque<PTR<NetworkBuffer>> m_datas{};

		int32_t m_timeoutMs{ 0 };
		int64_t m_checkTimeMs{ 0 };
		int64_t m_aliveTimeMs{ 0 };
		//unsigned char m_nonce[crypto_stream_chacha20_NONCEBYTES + 1]{};
		bool m_encrypted;
		bool m_nonceAssigned;

	public:
		NetworkHost() = delete;
		NetworkHost(const HostID_t& hostID);
		virtual ~NetworkHost();

	public:
		ENetworkHost GetHostType() const;
		HostID_t GetHostID() const;
		SOCKET GetSocket() const;
		std::string GetIP();
		int32_t GetPort() const;
		int32_t GetTimeoutMs() const;

		bool SetSocket(SOCKET sock);
		void SetHostValue(const int64_t value);

		NetworkCallback GetCallback();
		void ExecuteCallback(int32_t messageID, const PTR<google::protobuf::Message>& message);
		void PacketCallback(int32_t messageID, char* messageBuffer, int32_t messageSize);

	public:
		void RemoveTask(NetworkTask* task);
		bool IsPossibleSend();

	public:
		bool Update(int64_t currentTimeMs);
		void Refresh();

	public:
		void Connect(NetworkCallback callback, std::string ip, int32_t port);
		void Connected();
		void Listen(NetworkCallback callback, std::string ip, int32_t port, int32_t timeoutMs);
		void Accept(NetworkCallback callback, std::string ip, int32_t port, int32_t timeoutMs, SOCKET sock);
		void Send(PTR<NetworkBuffer> buffer = nullptr);
		void Close();

		bool IsEncrypted() const;
		void SetEncrypted();
		void SetNonce(unsigned char nonce[]);
		const unsigned char* FindNonce() const;
	};
}