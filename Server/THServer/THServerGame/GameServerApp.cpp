#include "GamePch.h"
#include "GameServerApp.h"
#include "ConfigSource.h"
#include "Configuration.h"
#include "ConsoleChannel.h"
#include "FileChannel.h"
#include "OutGameService.h"

namespace th
{
	GameServerApp::GameServerApp()
	{
	}

	GameServerApp::~GameServerApp()
	{
		Exit();
	}

	void GameServerApp::Start()
	{
		if (!InitLogger()) return;
		if (!LoadConfig()) return;

		// TODO
		//if (!LoadDBSetting()) return;
		//if (!LoadServerConfig()) return;
		//if (!InitConcurrentSystem()) return;
		//RunConcurrentSystem()
		//if (!LoadData()) return;

		InitSingleton();
		// TODO
		//if (!InitRedis()) return;

		RunLoop();
	}

	bool GameServerApp::RunLoop()
	{
		// TOOD DB
		OutGameService::GetInstance().Start();

		BindEventSink([](const PTR<PacketWrapper>& msg) { TH_LOG_ERROR(0, 0, "SendToLogicService(msg)"); });

		const auto profile = &ServiceProfile::GetInstance();
		if (!CreateNetWorkSocket(*profile)) return false;

		Run(*profile);
		return true;
	}

	bool GameServerApp::InitLogger()
	{
		auto& logger = th::Logger::GetInstance();
		logger.AddChannel(UNIQUE_NEW(th::ConsoleChannel));
		logger.AddChannel(UNIQUE_NEW(th::FileChannel, "../log", "Game"));
		logger.SetMinLevel(th::ELogLevel::Trace);
		logger.Start();
		return true;
	}

	bool GameServerApp::LoadConfig()
	{
		auto& config = Configuration::GetInstance();
		auto paths = ConfigSource::MakeInitialConfigFullPaths();
		if (!config.Load(paths)) return false;

		auto& profile = ServiceProfile::GetInstance();
		profile.Load(config);
		return true;
	}

	void GameServerApp::LoadLogLevel()
	{
		const auto& logLevel = static_cast<ELogLevel>(Configuration::GetInstance().Get("Log", "Level").ToInt());
		Logger::GetInstance().SetMinLevel(logLevel);
	}

	bool GameServerApp::LoadData()
	{
		return true;
	}

	bool GameServerApp::LoadDBSetting()
	{
		return true;
	}

	bool GameServerApp::LoadServerConfig()
	{
		return true;
	}

	bool GameServerApp::InitRedis()
	{
		return true;
	}

	void GameServerApp::InitSingleton()
	{

	}

	void GameServerApp::Update()
	{
		if (OutGameService::GetInstance().IsCrash())
		{
			ForceCrash();
		}

		if (OutGameService::GetInstance().IsQuit())
		{
			HANDLE hCurrentProcess = GetCurrentProcess();
			TerminateProcess(hCurrentProcess, ~0u);
		}
	}

	void GameServerApp::Exit()
	{
		if (m_exit) return;
		m_exit = true;

		Stop();
		Logger::GetInstance().Stop();
		OutGameService::GetInstance().Stop();
	}
}
