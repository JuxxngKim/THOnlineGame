#include "DummyPch.h"
#include "DummyClient.h"

int main(int argc, char* argv[])
{
	// 기본값
	std::string serverIP = "127.0.0.1";
	int32_t serverPort = 35501;
	std::string pid = "dummy_test_001";
	std::string authToken = "dummy_auth_token";

	// 커맨드 라인 인자 처리
	// Usage: THDummyClient.exe [ip] [port] [pid] [authToken]
	if (argc >= 2) serverIP = argv[1];
	if (argc >= 3) serverPort = std::atoi(argv[2]);
	if (argc >= 4) pid = argv[3];
	if (argc >= 5) authToken = argv[4];

	dummy::DummyClient client;

	// 초기화
	if (!client.Initialize(serverIP, serverPort, pid, authToken))
	{
		std::cout << "[FATAL] Failed to initialize DummyClient." << std::endl;
		return -1;
	}

	// 서버 접속
	client.Connect();

	// 메인 루프 (q 입력 시 종료)
	client.Run();

	// 정리
	client.Shutdown();

	return 0;
}
