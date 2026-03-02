#pragma once

#include "NetworkHost.h"
#include "NetworkEvent.h"
#include "DefineEnum.h"

namespace network
{
	class NetworkManager : public Singleton<NetworkManager>
	{
	private:
		volatile HANDLE m_iocp;
		std::list<std::thread> m_threads;

		std::recursive_mutex m_serializerLock;
		std::deque<PTR<NetworkEventSend>> m_serializerQueue;

		std::recursive_mutex m_controllerLock;
		bool m_controllerQueueFlag;
		std::deque<PTR<NetworkEvent>> m_controllerQueue1;
		std::deque<PTR<NetworkEvent>> m_controllerQueue2;

		int64_t m_nextUpdateHostTimeMs;
		HostID_t m_lastHostID;
		HostID_t m_dummyLastHostID;
		std::unordered_map<HostID_t, PTR<NetworkHost>> m_hosts;
		int32_t m_hostCount;

		volatile th::ETraceMessageLevel m_traceLevel;

	public:
		NetworkManager();
		virtual ~NetworkManager();

	public:
		//network
		void CreateNetwork();
		void DestroyNetwork();

	public:
		//worker
		bool WorkerBind(SOCKET sock) const;
		bool WorkerPush(NetworkTask* task) const;
		void WorkerProcess() const;

	public:
		//serializer
		void SerializerPush(const PTR<NetworkEventSend>& context);
		NetworkEventSend* SerializerPop();
		void SerializerProcess();

	public:
		//controller
		void ControllerPush(const PTR<NetworkEvent>& evt);
		std::deque<PTR<NetworkEvent>>& ControllerPop();
		void ControllerProcess();

	public:
		//host
		NetworkHost* CreateHost();
		NetworkHost* FindHost(const HostID_t& hostID);
		void UpdateHost();

		int32_t GetHostCount() const;

	public:
		//trace
		th::ETraceMessageLevel GetTraceMessageLevel() const;
		void SetTraceMessageLevel(th::ETraceMessageLevel Level);
		bool TraceMessage(std::string& log, int32_t messageID, const google::protobuf::Message& message) const;

	public:
		//request
		void Connect(NetworkCallback callback, std::string ip, int32_t port);
		void Listen(NetworkCallback callback, std::string ip, int32_t port, int32_t timeoutMs);
		void Send(std::unordered_set<HostID_t>& hostIds, int32_t messageID, const PTR<google::protobuf::Message>& message, const bool&
			ignoreEncrypt = false);
		void Send(const HostID_t& hostID, int32_t messageID, const PTR<google::protobuf::Message>& message, const bool& ignoreEncrypt = false);
		void Close(std::unordered_set<HostID_t>& hostIds);
		void Close(const HostID_t& hostID);
		void RegisterHostValue(const HostID_t& hostID, const int64_t value);
		void SetEncrypt(const HostID_t& hostID);
		void SetNonce(const HostID_t& hostID, const unsigned char nonce[]);
		HostID_t FindDummyHostID();
	};
}