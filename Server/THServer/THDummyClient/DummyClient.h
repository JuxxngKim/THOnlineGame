#pragma once

namespace dummy
{
	// 더미 클라이언트 상태
	enum class EClientState
	{
		None = 0,
		Connecting,		// 서버 접속 시도 중
		Connected,		// TCP 연결 완료, NetConnect 수신 대기
		LoggingIn,		// CALoginReq 전송 완료, ACLoginAck 대기
		LoggedIn,		// 로그인 성공, 세션 유지 중
		Disconnected,	// 연결 종료
	};

	class DummyClient
	{
	private:
		// 접속 정보
		std::string m_serverIP;
		int32_t m_serverPort;

		// 로그인 정보
		std::string m_pid;
		std::string m_authToken;

		// 상태
		EClientState m_state{ EClientState::None };
		HostID_t m_hostID{ 0 };
		th::AccountUID_t m_accountUID{ 0 };

		// 종료 플래그
		std::atomic<bool> m_running{ false };

	public:
		DummyClient() = default;
		~DummyClient() = default;

		// 초기화
		bool Initialize(const std::string& ip, int32_t port, const std::string& pid, const std::string& authToken);

		// 서버 접속
		void Connect();

		// 메인 루프 (콘솔 입력 처리 + NetworkManager 구동)
		void Run();

		// 종료
		void Shutdown();

		// 상태 조회
		EClientState GetState() const;
		bool IsRunning() const;

	private:
		// 패킷 수신 콜백 (NetworkManager에서 호출)
		void OnPacketReceived(const PTR<th::PacketWrapper>& packet);

		// 패킷 핸들러
		void HandleNetConnect(const PTR<th::PacketWrapper>& packet);
		void HandleNetDisconnect(const PTR<th::PacketWrapper>& packet);
		void HandleACLoginAck(const PTR<th::PacketWrapper>& packet);
		void HandleACCommonNak(const PTR<th::PacketWrapper>& packet);

		// CALoginReq 전송
		void SendLoginRequest();

		// 상태 변경 로그
		void SetState(EClientState newState);
	};
}
