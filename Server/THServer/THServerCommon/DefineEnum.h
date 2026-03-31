#pragma once

namespace th
{
	enum class EPlayerType
	{
		None,
		NewPlayer, // 신규
		NormalPlayer, // 일반
		ReturnPlayer, // 복귀
		Max,
	};

	enum class ELoginData
	{
		Player = 0,
		Max,
	};

	enum class ETraceMessageLevel
	{
		Disable = 0,
		HighLevel = 1,
		LowLevel = 2,
	};

	enum class EServer
	{
		None = 0,
		Gateway = 1,
		Game = 2,
	};

	enum class ELogLevel : uint8_t
	{
		None = 0,
		Trace = 0,
		Debug = 1,
		Info = 2,
		Warn = 3,
		Error = 4,
		Critical = 5,
		Off = 6,
	};

	enum ELogicEvent : uint8_t
	{
		None = 0,
		Prepare = 1,
		Arrange = 2,
	};

	enum class ELanguageCode
	{
		None = 0,
		Korea = 1,
		Japan = 2,
		HanSimplified = 3,
		HanTraditional = 4,
		Thai = 5,
		English = 6,
		Indonesia = 7,
		Max
	};

	enum class EDBSession
	{
		None = 0,
		Membership = 1,
		Game = 2,
	};

	enum class EPlayerStatus
	{
		None = 0,
		Play = 1,
		Idle = 2,
		EndofGameSession = 3,
	};

	enum class EPlayerDisconnectionStatus
	{
		None = 0,
		LoginBan = 1,
		AccountWithdrawal = 2,
		SequenceMaintenance = 3,    // 순차점검(무중단점검)
		Maintenance = 4,    // 정기점검
		AccountWithdrawalFromWeb = 5,    // 웹페이지를 통한 탈퇴
		Max,
	};
}