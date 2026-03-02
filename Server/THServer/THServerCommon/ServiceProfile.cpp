#include "CommonPch.h"
#include "ServiceProfile.h"
#include "Configuration.h"

namespace th
{
	ServiceProfile::ServiceProfile()
		: m_serverId(0)
		, m_worldId(0)
		, m_timeoutMs(0)
	{}

	void ServiceProfile::Load(const Configuration& config)
	{
		m_name = config.Get("Profile", "Service").ToLower();
		m_serverId = config.Get("Profile", "ID").Default(1);

		auto section = FindQualifiedName();

		m_bindAddress = config.Get(section, "BindAddr").ToIpEndPoint();
		m_advertiseAddress = config.Get(section, "AdvertiseAddr").ToIpEndPoint();

		m_worldId = config.Get("World", "ID");

		auto timeoutConfig = config.Get(section, "NetworkTimeoutMS");
		m_timeoutMs = timeoutConfig.Valid() ? timeoutConfig : config.Get(m_name, "NetworkTimeoutMS");

		m_environment = config.Get("Profile", "Env").Default(std::string{ "local" });
		m_dumpAddress = config.Get("DumpServer", "ServerAddr").ToIpEndPoint();
	}

	void ServiceProfile::SetBindAddress(const std::string& address)
	{
		m_bindAddress = IpEndPoint(address);
	}

	void ServiceProfile::SetAdvertiseAddress(const std::string& address)
	{
		m_advertiseAddress = IpEndPoint(address);
	}

	void ServiceProfile::SetTimeoutMs(int timeoutMs)
	{
		m_timeoutMs = timeoutMs;
	}

	std::string ServiceProfile::FindName() const
	{
		return m_name;
	}

	std::string ServiceProfile::FindQualifiedName() const
	{
		return m_name + "." + std::to_string(m_serverId);
	}

	EServer ServiceProfile::FindType() const
	{
		if (m_name == "gateway") return EServer::Gateway;
		if (m_name == "game") return EServer::Game;
		return EServer::None;
	}

	int ServiceProfile::FindServerID() const
	{
		return m_serverId;
	}

	std::string ServiceProfile::FindAdvertiseAddress() const
	{
		return m_advertiseAddress.Hostname();
	}

	int ServiceProfile::FindAdvertisePort() const
	{
		return m_advertiseAddress.Port();
	}

	std::string ServiceProfile::FindBindAddress() const
	{
		return m_bindAddress.Hostname();
	}

	int ServiceProfile::FindBindPort() const
	{
		return m_bindAddress.Port();
	}

	int ServiceProfile::FIndWorldId() const
	{
		return m_worldId;
	}

	int ServiceProfile::FindTimeoutMs() const
	{
		return m_timeoutMs;
	}

	std::string ServiceProfile::FindDumpIP() const
	{
		return m_dumpAddress.Hostname();
	}

	int ServiceProfile::FindDumpPort() const
	{
		return m_dumpAddress.Port();
	}

	std::string ServiceProfile::FindEnvironment() const
	{
		return m_environment;
	}

	bool ServiceProfile::IsLocal() const
	{
		return m_environment == "local";
	}
}
