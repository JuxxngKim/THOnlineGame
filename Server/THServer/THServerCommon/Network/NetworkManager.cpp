#include "CommonPch.h"
#include "ThreadCollector.h"
#include "NetworkManager.h"
//#include "PacketLogFilter.h"

namespace network
{
	NetworkManager::NetworkManager()
		: m_iocp(INVALID_HANDLE_VALUE)
		  , m_controllerQueueFlag(false)
		  , m_nextUpdateHostTimeMs(0)
		  , m_lastHostID(MIN_HOST_ID)
		  , m_dummyLastHostID(0)
	      , m_hostCount(0)
		  , m_traceLevel(th::ETraceMessageLevel::Disable)
	{
		WSADATA wsa = {};
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}

	NetworkManager::~NetworkManager()
	{
		DestroyNetwork();

		for (auto& thread : m_threads)
		{
			if (thread.joinable()) thread.join();
		}

		m_threads.clear();

		WSACleanup();
	}

	//network
	void NetworkManager::CreateNetwork()
	{
		//iocp핸들 생성
		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
		if (m_iocp == nullptr || m_iocp == INVALID_HANDLE_VALUE)
		{
			TH_LOG_ERROR(0, 0, "failed - CreateIoCompletionPort : [err:%]", GetLastError());
			m_iocp = INVALID_HANDLE_VALUE;
			return;
		}
		//스레드 생성
		for (int32_t n = 0; n < MAX_WORKER_COUNT; ++n)
		{
			m_threads.emplace_back([this] { WorkerProcess(); });
		}

		for (int32_t n = 0; n < MAX_SERIALIZER_COUNT; ++n)
		{
			m_threads.emplace_back([this] { SerializerProcess(); });
		}

		m_threads.emplace_back([this] { ControllerProcess(); });

		//생성대기
		Sleep(100);
	}

	void NetworkManager::DestroyNetwork()
	{
		if (m_iocp != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_iocp);
			m_iocp = INVALID_HANDLE_VALUE;
		}
	}

	//worker
	bool NetworkManager::WorkerBind(SOCKET sock) const
	{
		if (CreateIoCompletionPort((HANDLE)sock, m_iocp, 0, 0) == nullptr)
		{
			TH_LOG_ERROR(0, 0, "CreateIoCompletionPort failed : [err:%]", GetLastError());
			return false;
		}

		return true;
	}

	bool NetworkManager::WorkerPush(NetworkTask* task) const
	{
		if (PostQueuedCompletionStatus(m_iocp, 0, (ULONG_PTR)task, 0) == FALSE)
		{
			TH_LOG_ERROR(0, 0, "PostQueuedCompletionStatus failed : [err:%]", GetLastError());
			return false;
		}

		return true;
	}

	void NetworkManager::WorkerProcess() const
	{
		std::string tname("NetworkWorker");
		int32_t threadID = GetCurrentThreadId();
		util::ThreadCollector::InsertThreadInfo(tname, threadID);

		bool result = true;
		DWORD transferred = 0;
		ULONG_PTR key = 0;
		LPOVERLAPPED overlapped = 0;
		NetworkTask* task = nullptr;

		//스레드 작업 실행
		while (m_iocp != INVALID_HANDLE_VALUE)
		{
			//이벤트 대기
			result = true;
			transferred = 0;
			key = 0;
			overlapped = 0;

			if (GetQueuedCompletionStatus(m_iocp, &transferred, &key, &overlapped, INFINITE) == FALSE)
			{
				if (GetLastError() != ERROR_NETNAME_DELETED)
				{
					result = false;
				}
			}

			task = (NetworkTask*)key;
			if (task)
			{
				task->Prepare();
				continue;
			}

			task = (NetworkTask*)overlapped;
			if (task)
			{
				task->Complete(result, (int32_t)transferred);
				continue;
			}
		}
	}

	//serializer
	void NetworkManager::SerializerPush(const PTR<NetworkEventSend>& context)
	{
		if (context == nullptr) return;

		std::lock_guard<std::recursive_mutex> guard(m_serializerLock);

		m_serializerQueue.emplace_back(context);
	}

	NetworkEventSend* NetworkManager::SerializerPop()
	{
		std::lock_guard<std::recursive_mutex> guard(m_serializerLock);

		//앞에서부터 차례대로 완료된게 있으면 보낸다
		while (!m_serializerQueue.empty())
		{
			//NOTE::ryj const & X, 아래에 pop_front로 제거 하기 때문에 ref카운트를 늘리기 위해서 const & 사용 안함.
			auto evt = m_serializerQueue.front();
			if (evt->m_step != ESerialize::Completed) break;

			m_serializerQueue.pop_front();

			ControllerPush(evt);
		}

		//처리할 대상 찾기
		for (auto& evt : m_serializerQueue)
		{
			if (evt->m_step != ESerialize::None) continue;

			evt->m_step = ESerialize::Started;
			return evt.get();
		}

		return nullptr;
	}

	void NetworkManager::SerializerProcess()
	{
		std::string tname("NetworkSerializer");
		int32_t threadID = ::GetCurrentThreadId();
		util::ThreadCollector::InsertThreadInfo(tname, threadID);

		while (m_iocp != INVALID_HANDLE_VALUE)
		{
			//이벤트 처리
			auto evt = SerializerPop();
			if (evt == nullptr)
			{
				Sleep(1);
				continue;
			}

			evt->Serialize();
		}
	}

	//controller
	void NetworkManager::ControllerPush(const PTR<NetworkEvent>& evt)
	{
		std::lock_guard<std::recursive_mutex> guard(m_controllerLock);

		if (m_controllerQueueFlag)
		{
			m_controllerQueue1.emplace_back(evt);
		}
		else
		{
			m_controllerQueue2.emplace_back(evt);
		}
	}

	std::deque<PTR<NetworkEvent>>& NetworkManager::ControllerPop()
	{
		std::lock_guard<std::recursive_mutex> guard(m_controllerLock);

		if (m_controllerQueueFlag)
		{
			m_controllerQueueFlag = false;
			return m_controllerQueue1;
		}

		m_controllerQueueFlag = true;
		return m_controllerQueue2;
	}

	void NetworkManager::ControllerProcess()
	{
		std::string tname("NetworkController");
		int32_t threadID = GetCurrentThreadId();
		util::ThreadCollector::InsertThreadInfo(tname, threadID);

		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

		while (m_iocp != INVALID_HANDLE_VALUE)
		{
			UpdateHost();

			//이벤트 처리
			auto& evts = ControllerPop();
			if (evts.empty() == false)
			{
				for (auto& evt : evts)
				{
					evt->Run();
				}

				evts.clear();
			}

			Sleep(1);
		}
	}

	//host
	NetworkHost* NetworkManager::CreateHost()
	{
		//등록
		while (true)
		{
			++m_lastHostID;

			if (m_lastHostID > MAX_HOST_ID)
			{
				m_lastHostID = MIN_HOST_ID;
			}

			//맵에 등록한다
			if (m_hosts.contains(m_lastHostID))
			{
				continue;
			}

			auto host = NEW(NetworkHost, m_lastHostID);
			m_hosts.insert({ m_lastHostID, host });
			return host.get();
		}

		return nullptr;
	}

	NetworkHost* NetworkManager::FindHost(const HostID_t& hostID)
	{
		auto iter = m_hosts.find(hostID);
		if (iter == m_hosts.end()) return nullptr;

		return iter->second.get();
	}

	void NetworkManager::UpdateHost()
	{
		int64_t currentTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs();
		if (m_nextUpdateHostTimeMs > currentTimeMs) return;

		auto iter = m_hosts.begin();
		while (iter != m_hosts.end())
		{
			auto host = iter->second.get();
			if (host == nullptr)
			{
				iter = m_hosts.erase(iter);
				continue;
			}

			if (host->Update(currentTimeMs) == false)
			{
				//종료이벤트 호출
				auto msg = NEW(th::NetDisconnect);
				host->ExecuteCallback(msg->messageid(), msg);

				iter = m_hosts.erase(iter);
				continue;
			}

			++iter;
		}

		m_nextUpdateHostTimeMs = util::TimeUtil::GetInstance().FindTickKSTMs() + UPDATE_HOST_INTERVAL_MS;
		m_hostCount = (int32_t)m_hosts.size();
	}

	int32_t NetworkManager::GetHostCount() const
	{
		return m_hostCount;
	}

	//trace
	th::ETraceMessageLevel NetworkManager::GetTraceMessageLevel() const
	{
		return m_traceLevel;
	}

	void NetworkManager::SetTraceMessageLevel(th::ETraceMessageLevel Level)
	{
		m_traceLevel = Level;

		if (m_traceLevel == th::ETraceMessageLevel::HighLevel)
		{
			TH_LOG_ERROR(0, 0, "high level trace message ON");
		}
		else if (m_traceLevel == th::ETraceMessageLevel::LowLevel)
		{
			TH_LOG_ERROR(0, 0, "low level trace message ON");
		}
		else
		{
			TH_LOG_ERROR(0, 0, "trace message off");
		}

		return;
	}

	bool NetworkManager::TraceMessage(std::string& log, int32_t messageID, const google::protobuf::Message& message) const
	{
		if (m_traceLevel == th::ETraceMessageLevel::Disable) return false;

		if (m_traceLevel == th::ETraceMessageLevel::LowLevel)	//필터링
		{
			switch (messageID)
			{
			case th::NET_ALIVE_REQ:
			case th::NET_ALIVE_ACK:

				return true;
			}
		}

		//log = StrUtil::ToJson(message);

		return true;
	}

	//request
	void NetworkManager::Connect(NetworkCallback callback, std::string ip, int32_t port)
	{
		auto ncc = NEW(NetworkEventConnect);
		ncc->m_callback = callback;
		ncc->m_ip = ip;
		ncc->m_port = port;
		ControllerPush(ncc);
	}

	void NetworkManager::Listen(NetworkCallback callback, std::string ip, int32_t port, int32_t timeoutMs)
	{
		auto ncc = NEW(NetworkEventListen);
		ncc->m_callback = callback;
		ncc->m_ip = ip;
		ncc->m_port = port;
		ncc->m_timeoutMs = timeoutMs;
		ControllerPush(ncc);
	}

	void NetworkManager::Send(std::unordered_set<HostID_t>& hostIds, int32_t messageID, const PTR<google::protobuf::Message>& message, const bool& ignoreEncrypt)
	{
		hostIds.erase(0);
		if (hostIds.empty() == true) return;

		//for (const auto& hostID : hostIds)
		//{
		//	util::PacketLogFilter::Write(hostID, 0, "netmgr send", static_cast<re::EMessageID>(messageID), *message.get());
		//}

		auto ncc = NEW(NetworkEventSend);
		ncc->m_hostIds.swap(hostIds);
		ncc->m_packetId = messageID;
		ncc->m_packet = message;
		ncc->m_ignoreEncrypt = ignoreEncrypt;
		SerializerPush(ncc);

		//CommonMetric::GetInstance().IncrementPacketSendCounter(messageID);
	}

	void NetworkManager::Send(const HostID_t& hostID, int32_t messageID, const PTR<google::protobuf::Message>& message, const bool& ignoreEncrypt)
	{
		std::unordered_set<HostID_t> hostIds;
		hostIds.insert(hostID);
		Send(hostIds, messageID, message, ignoreEncrypt);
	}

	void NetworkManager::Close(std::unordered_set<HostID_t>& hostIds)
	{
		hostIds.erase(0);
		if (hostIds.empty() == true)
			return;

		auto ncc = NEW(NetworkEventClose);
		ncc->m_hostIds.swap(hostIds);
		ControllerPush(ncc);
	}

	void NetworkManager::Close(const HostID_t& hostID)
	{
		if (hostID == 0) return;

		std::unordered_set<HostID_t> hostIds;
		hostIds.insert(hostID);
		Close(hostIds);
	}

	void NetworkManager::RegisterHostValue(const HostID_t& hostID, const int64_t value)
	{
		auto ncc = NEW(NetworkEventRegisterValue);
		ncc->m_hostId = hostID;
		ncc->m_value = value;
		ControllerPush(ncc);
	}

	void NetworkManager::SetEncrypt(const HostID_t& hostID)
	{
		const auto& ncc = NEW(NetworkEventEncrypt);
		ncc->m_hostID = hostID;
		ControllerPush(ncc);
	}

	void NetworkManager::SetNonce(const HostID_t& hostID, const unsigned char nonce[])
	{
		const auto& ncc = NEW(NetworkEventNonce);
		ncc->m_hostID = hostID;
		//memcpy_s(ncc->m_nonce, crypto_stream_chacha20_NONCEBYTES, nonce, crypto_stream_chacha20_NONCEBYTES);
		ControllerPush(ncc);
	}

	HostID_t NetworkManager::FindDummyHostID()
	{
		return --m_dummyLastHostID;
	}
}