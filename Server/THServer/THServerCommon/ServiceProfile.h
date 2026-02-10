#pragma once

#include "IpEndPoint.h"

namespace th
{
	class Configuration;

	class ServiceProfile : public Singleton<ServiceProfile>
	{
	private:
		std::string m_name;
		int m_serverId;
		IpEndPoint m_advertiseAddress;
		IpEndPoint m_bindAddress;
		IpEndPoint m_dumpAddress;
		int m_worldId;

		int m_timeoutMs;
		std::string m_environment;

	public:
		ServiceProfile();

		void Load(const Configuration& config);
		void SetBindAddress(const std::string& address);
		void SetAdvertiseAddress(const std::string& address);
		void SetTimeoutMs(int timeoutMs);

		std::string FindName() const;
		std::string FindQualifiedName() const;
		th::EServer FindType() const;
		int FindServerID() const;
		std::string FindAdvertiseAddress() const;
		int FindAdvertisePort() const;
		std::string FindBindAddress() const;
		int FindBindPort() const;
		int FIndWorldId() const;

		int FindTimeoutMs() const;
		std::string FindDumpIP() const;
		int FindDumpPort() const;

		std::string FindEnvironment() const;
		bool IsLocal() const;
	};
}