#include "CommonPch.h"
#include "ServerApp.h"
#include "ServiceProfile.h"
#include "ThreadCollector.h"

namespace th
{
	ServerApp::ServerApp()
		: m_callback(nullptr)
		, m_running(false)
	{
		RegisterCommand('q', std::bind(&ServerApp::ProcessQuit, this));
	}

	ServerApp::~ServerApp()
	{
	}

	void ServerApp::Run(const ServiceProfile& profile)
	{
		if (Start(profile))
		{
			auto name = profile.FindName();
			TH_LOG_INFO(0, 0, "server waiting for keyboard-server : name:%", name);

			m_running = true;
			while (m_running)
			{
				if (_kbhit())
				{
					auto iter = m_commandMap.find(_getch());
					if (iter != m_commandMap.end())
					{
						(iter->second)();
					}
				}

				Update();
				Sleep(1000);
			}

			TH_LOG_INFO(0, 0, "server closing : [server:%]", name);
		}

		Stop();
	}

	bool ServerApp::Start(const ServiceProfile& profile)
	{
		auto name = profile.FindName();
		auto ip = profile.FindBindAddress();
		auto port = profile.FindBindPort();
		//TODO DUMP
		//dump::MiniDump::SetupDumpServer(profile.FindDumpIP(), profile.FindDumpPort());
		//dump::MiniDump::SetupDumpName(ip);

		TH_LOG_INFO(0, 0, "info : server:%, host:%, port:%", name, ip, port);
		TH_LOG_INFO(0, 0, "server started : server:%", name);
		util::ThreadCollector::PrintThreadInfo();
		return true;
	}

	void ServerApp::Stop()
	{
		network::NetworkManager::GetInstance().DestroyNetwork();
	}

	bool ServerApp::CreateNetWorkSocket(const ServiceProfile& profile)
	{
		network::NetworkManager::GetInstance().CreateNetwork();

		if (nullptr == m_callback)
		{
			TH_LOG_ERROR(0, 0, "handler null :[server:%]", profile.FindName());
			return false;
		}

		auto name = profile.FindName();
		auto ip = profile.FindBindAddress();
		auto port = profile.FindBindPort();
		auto timeoutMs = profile.FindTimeoutMs();

		network::NetworkManager::GetInstance().Listen(m_callback, ip, port, timeoutMs);

		TH_LOG_INFO(0, 0, "Socket Listen  : [server:%]", name);

		return true;
	}

	void ServerApp::BindEventSink(network::NetworkCallback callback)
	{
		m_callback = callback;
	}

	void ServerApp::RegisterCommand(int32_t cmd, CommandFunc func)
	{
		auto iter = m_commandMap.find(cmd);
		if (iter != m_commandMap.end())
		{
			TH_LOG_ERROR(0, 0, "duplicated cmd : [cmd:%]", cmd);
			return;
		}

		m_commandMap[cmd] = func;
	}

	void ServerApp::ProcessQuit()
	{
		m_running = false;
	}

	void ServerApp::ForceCrash(int32_t divider)
	{
		TH_LOG_ERROR(0, 0, "%", 1 / divider);
	}
}
