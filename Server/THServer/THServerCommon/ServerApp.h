#pragma once

namespace th
{
	class ServiceProfile;

	class ServerApp
	{
	public:
		using CommandFunc = std::function<void(void)>;
		using SyncTimeFunc = std::function<bool()>;
		using CheckReconnectAssist = std::function<bool()>;
		using InitFunc = std::function<void(void)>;

	private:
		network::NetworkCallback m_callback;

		volatile bool m_running;
		std::unordered_map<int32_t, CommandFunc> m_commandMap;

	public:
		ServerApp();
		virtual ~ServerApp();

	private:
		void RegisterCommand(int32_t cmd, CommandFunc func);

	protected:
		void Stop();
		void ProcessQuit();
		static void ForceCrash(int32_t divider = 0);

	protected:
		virtual void Update() = 0;
		bool CreateNetWorkSocket(const ServiceProfile& profile);

		void BindEventSink(network::NetworkCallback callback);
		void Run(const ServiceProfile& profile);

		bool Start(const ServiceProfile& profile);
	};
}