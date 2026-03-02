#pragma once

namespace network
{
	using NetworkCallback = std::function<void(const PTR<th::PacketWrapper>&)>;

	const int32_t	MAX_SERIALIZER_COUNT = 4;	// 시리얼라이저 쓰레드 갯수
	const int32_t	MAX_WORKER_COUNT = 4;		// 워커 쓰레드 갯수

	const HostID_t	MIN_HOST_ID = 1;
	const HostID_t	MAX_HOST_ID = 2000000000;
	const int64_t	UPDATE_HOST_INTERVAL_MS = 5000;		// 워커 쓰레드 갯수

	const int32_t	MIN_CAPACITY_SIZE = 8 * 1024; //사용가능한 최소 사이즈
	const int32_t	DEFAULT_CAPACITY_SIZE = MIN_CAPACITY_SIZE * 2; //버퍼 생성 혹은 늘릴때 사이즈
	const int32_t	MAX_CAPACITY_SIZE = 1024 * 1024; // 1MB 패킷제한

	const int32_t	MAX_ACCEPT_COUNT = 100;

	const int32_t	MAX_ADDRESS_SIZE = (sizeof(SOCKADDR_IN) + 16) * 2;

	enum class ENetworkHost
	{
		None = 0,
		Connecter,
		Listener,
		Accepter,
	};

	enum class ENetworkTask
	{
		None = 0,
		Connect,
		Accept,
		Receive,
		Send,
	};

	enum class ENetworkEvent
	{
		None = 0,
		Task,
		Connect,
		Listen,
		Accept,
		Send,
		Close,
		RegisterValue,
		Encrypt,
		Nonce,
	};

	enum class ESerialize
	{
		None = 0,
		Started,
		Completed,
	};
}