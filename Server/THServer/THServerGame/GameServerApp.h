#pragma once
#include "ServerApp.h"

namespace th
{
	class GameServerApp : public ServerApp
	{
	private:
		bool m_exit{ false };

	public:
		GameServerApp();
		virtual ~GameServerApp() override;
		void Start();

	private:
		bool InitLogger();
		bool LoadConfig();
		void LoadLogLevel();
		bool LoadData();
		bool LoadDBSetting();
		bool LoadServerConfig();
		bool InitRedis();
		void InitSingleton();
		bool RunLoop();
		virtual void Update() override;
		void Exit();
	};
}
