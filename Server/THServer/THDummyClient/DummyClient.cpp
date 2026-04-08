#include "DummyPch.h"
#include "DummyClient.h"
#include "MessageCreator.h"

namespace dummy
{
	// 더미 클라이언트가 수신할 수 있는 메시지 타입 등록
	static void RegisterClientMessages()
	{

	}

	bool DummyClient::Initialize(const std::string& ip, int32_t port, const std::string& pid, const std::string& authToken)
	{
		m_serverIP = ip;
		m_serverPort = port;
		m_pid = pid;
		m_authToken = authToken;

		// NetworkManager 초기화 (Singleton 생성자에서 WSAStartup 호출, CreateNetwork에서 IOCP + 워커/시리얼라이저 스레드 생성)
		network::NetworkManager::GetInstance().CreateNetwork();

		// 클라이언트 수신 메시지 등록 (MessageCreator)
		RegisterClientMessages();

		m_running = true;
		SetState(EClientState::None);

		std::cout << "[INFO] DummyClient initialized." << std::endl;
		std::cout << "  Server: " << m_serverIP << ":" << m_serverPort << std::endl;
		std::cout << "  PID: " << m_pid << std::endl;

		return true;
	}

	void DummyClient::Connect()
	{
		SetState(EClientState::Connecting);

		// NetworkManager::Connect -> Connecter 타입 호스트 생성
		// Connecter 타입은 NetworkHost::Update()에서 자동으로 NetAliveReq를 전송
		auto callback = [this](const PTR<th::PacketWrapper>& packet)
		{
			OnPacketReceived(packet);
		};

		network::NetworkManager::GetInstance().Connect(callback, m_serverIP, m_serverPort);

		std::cout << "[INFO] Connecting to " << m_serverIP << ":" << m_serverPort << "..." << std::endl;
	}

	void DummyClient::Run()
	{
		std::cout << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << "  TH Dummy Client" << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << "  Commands:" << std::endl;
		std::cout << "    q - Quit" << std::endl;
		std::cout << "    s - Show status" << std::endl;
		std::cout << "    c - Connect to server" << std::endl;
		std::cout << "========================================" << std::endl;
		std::cout << std::endl;

		while (m_running)
		{
			// NetworkManager 컨트롤러 처리 (접속/종료 이벤트)
			network::NetworkManager::GetInstance().ControllerProcess();

			// NetworkManager 호스트 업데이트 (타임아웃 체크, NetAlive 전송)
			network::NetworkManager::GetInstance().UpdateHost();

			// 콘솔 입력 처리 (논블로킹)
			if (_kbhit())
			{
				const auto ch = _getch();
				switch (ch)
				{
				case 'q':
				case 'Q':
					std::cout << "[INFO] Quit requested." << std::endl;
					m_running = false;
					break;

				case 's':
				case 'S':
				{
					const char* stateStr = "Unknown";
					switch (m_state)
					{
					case EClientState::None:			stateStr = "None"; break;
					case EClientState::Connecting:		stateStr = "Connecting"; break;
					case EClientState::Connected:		stateStr = "Connected"; break;
					case EClientState::LoggingIn:		stateStr = "LoggingIn"; break;
					case EClientState::LoggedIn:		stateStr = "LoggedIn"; break;
					case EClientState::Disconnected:	stateStr = "Disconnected"; break;
					}
					std::cout << "[STATUS] State=" << stateStr
						<< ", HostID=" << m_hostID
						<< ", AccountUID=" << m_accountUID << std::endl;
					break;
				}

				case 'c':
				case 'C':
					if (m_state == EClientState::None || m_state == EClientState::Disconnected)
					{
						Connect();
					}
					else
					{
						std::cout << "[WARN] Already connected or connecting." << std::endl;
					}
					break;

				default:
					break;
				}
			}

			// CPU 과점유 방지
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void DummyClient::Shutdown()
	{
		m_running = false;

		// 연결 종료
		if (m_hostID != 0)
		{
			network::NetworkManager::GetInstance().Close(m_hostID);
		}

		// NetworkManager 정리 (소멸자에서 WSACleanup 호출)
		network::NetworkManager::GetInstance().DestroyNetwork();

		std::cout << "[INFO] DummyClient shutdown complete." << std::endl;
	}

	EClientState DummyClient::GetState() const
	{
		return m_state;
	}

	bool DummyClient::IsRunning() const
	{
		return m_running;
	}

	void DummyClient::OnPacketReceived(const PTR<th::PacketWrapper>& packet)
	{
		if (packet == nullptr) return;

		const auto messageID = packet->MessageID;

		switch (messageID)
		{
		case th::NET_CONNECT:
			HandleNetConnect(packet);
			break;

		case th::NET_DISCONNECT:
			HandleNetDisconnect(packet);
			break;

		case th::AC_LOGIN_ACK:
			HandleACLoginAck(packet);
			break;

		case th::AC_COMMON_NAK:
			HandleACCommonNak(packet);
			break;

		default:
			std::cout << "[RECV] MessageID=" << messageID << " (unhandled)" << std::endl;
			break;
		}
	}

	void DummyClient::HandleNetConnect(const PTR<th::PacketWrapper>& packet)
	{
		m_hostID = packet->HostID;

		const auto msg = std::dynamic_pointer_cast<th::NetConnect>(packet->Msg);
		if (msg)
		{
			std::cout << "[RECV] NetConnect - IP=" << msg->connectip()
				<< ", Port=" << msg->connectport()
				<< ", TimeoutMS=" << msg->timeoutms()
				<< ", ServerTime=" << msg->servertime() << std::endl;
		}

		SetState(EClientState::Connected);

		// 연결 완료 후 즉시 로그인 요청
		SendLoginRequest();
	}

	void DummyClient::HandleNetDisconnect(const PTR<th::PacketWrapper>& packet)
	{
		std::cout << "[RECV] NetDisconnect - HostID=" << packet->HostID << std::endl;

		m_hostID = 0;
		m_accountUID = 0;
		SetState(EClientState::Disconnected);
	}

	void DummyClient::HandleACLoginAck(const PTR<th::PacketWrapper>& packet)
	{
		const auto msg = std::dynamic_pointer_cast<th::ACLoginAck>(packet->Msg);
		if (msg == nullptr)
		{
			std::cout << "[ERROR] ACLoginAck parse failed." << std::endl;
			return;
		}

		m_accountUID = msg->accountid();

		std::cout << "[RECV] ACLoginAck - Login Success!" << std::endl;
		std::cout << "  AccountID=" << msg->accountid() << std::endl;
		std::cout << "  AccountName=" << msg->accountname() << std::endl;
		std::cout << "  IsReconnect=" << msg->isreconnect() << std::endl;
		std::cout << "  IsNewAccount=" << msg->isnewaccount() << std::endl;
		std::cout << "  ServerID=" << msg->serverid() << std::endl;
		std::cout << "  ChannelID=" << msg->channelid() << std::endl;
		std::cout << "  Version=" << msg->version() << std::endl;

		SetState(EClientState::LoggedIn);

		// 로그인 성공 후 NetAlive는 Connecter 타입 호스트가 자동 처리
		std::cout << "[INFO] Session alive. NetAliveReq will be sent every "
			<< NETWORK_ALIVE_MS / 1000 << "s automatically." << std::endl;
	}

	void DummyClient::HandleACCommonNak(const PTR<th::PacketWrapper>& packet)
	{
		const auto msg = std::dynamic_pointer_cast<th::ACCommonNak>(packet->Msg);
		if (msg == nullptr)
		{
			std::cout << "[ERROR] ACCommonNak parse failed." << std::endl;
			return;
		}

		std::cout << "[RECV] ACCommonNak - Login Failed!" << std::endl;
		std::cout << "  Error=" << th::EErrorMsg_Name(msg->error()) << " (" << msg->error() << ")" << std::endl;
		std::cout << "  SuccessfulMsgId=" << msg->successfulmsgid() << std::endl;

		SetState(EClientState::Connected);
	}

	void DummyClient::SendLoginRequest()
	{
		auto loginReq = NEW(th::CALoginReq);
		loginReq->set_currentversion(th::RE_PROTOCOL_VERSION);
		loginReq->set_pid(m_pid);
		loginReq->set_languageid(1); // Korea
		loginReq->set_isreconnect(false);
		loginReq->set_authtoken(m_authToken);
		loginReq->set_platformtype(th::EPlatformType::NONE);
		loginReq->set_appversion("1.0.0");
		loginReq->set_ip("127.0.0.1");
		loginReq->set_sequencemaintenancelogin(false);

		network::NetworkManager::GetInstance().Send(m_hostID, loginReq->messageid(), loginReq);

		SetState(EClientState::LoggingIn);

		std::cout << "[SEND] CALoginReq - PID=" << m_pid
			<< ", Version=" << th::RE_PROTOCOL_VERSION << std::endl;
	}

	void DummyClient::SetState(EClientState newState)
	{
		m_state = newState;
	}
}
