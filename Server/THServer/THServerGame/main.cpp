#include "GamePch.h"
#include <iostream>

#include "ConfigSource.h"
#include "Configuration.h"
#include "ConsoleChannel.h"
#include "FileChannel.h"
#include "ServiceProfile.h"

void InitLogger()
{
	auto& logger = th::Logger::GetInstance();
	logger.AddChannel(UNIQUE_NEW(th::ConsoleChannel));
	logger.AddChannel(UNIQUE_NEW(th::FileChannel, "../log", "THServer"));
	logger.SetMinLevel(th::ELogLevel::Trace);
	logger.Start();
}

void ShutdownLogger()
{
	th::Logger::GetInstance().Shutdown();
}

void GameLogicExample()
{
	TH_LOG_INFO(0, 0, "Server started on port %", 8080);
	TH_LOG_DEBUG(724, 2181, "Config loaded: maxPlayers=%, tickRate=%", 100, 60);
	TH_LOG_WARN(724, 2181, "Player has high latency: %ms", 350);
	TH_LOG_ERROR(724, 2181, "message process time is over : [msgId:%, time:%]", 30364, 330);
	TH_LOG_CRITICAL(0, 0, "Database connection lost!");
}

void MultiThreadExample()
{
	std::vector<std::thread> threads;
	for (int i = 0; i < 4; ++i)
	{
		threads.emplace_back([i]()
			{
				for (int j = 0; j < 10; ++j)
				{
					TH_LOG_DEBUG(i, i * 100, "Thread % - message %", i, j);
				}
			});
	}
	for (auto& t : threads) t.join();
}

int32_t main(int arge, const char** argv)
{
	auto& config = th::Configuration::GetInstance();
	auto paths = th::ConfigSource::MakeInitialConfigFullPaths();
	if (!config.Load(paths)) return 0;

	auto& profile = th::ServiceProfile::GetInstance();
	profile.Load(config);

	InitLogger();

	TH_LOG_INFO(0, 0, "=== Logger System Example ===");
	GameLogicExample();
	MultiThreadExample();
	TH_LOG_INFO(0, 0, "=== Shutdown ===");

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
		TH_LOG_INFO(0, 0, "=== running ===");
	}
	
	return 0;
}