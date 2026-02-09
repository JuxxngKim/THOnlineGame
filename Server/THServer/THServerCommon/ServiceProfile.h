#pragma once

#include "IniConfig.h"
#include "IpEndPoint.h"

#include <string>
#include <cstdint>

namespace th
{
	class ServiceProfile : public Singleton<ServiceProfile>
	{
	private:
		IniConfig   m_config;

		std::string m_name;
		int32_t     m_serverId;
		std::string m_environment;
		int32_t     m_worldId;
		int32_t     m_timeoutMs;

		IpEndPoint  m_bindAddress;
		IpEndPoint  m_advertiseAddress;
		IpEndPoint  m_dumpAddress;

	public:
		ServiceProfile();

		bool Load(const std::string& configDir = "");
		
		const IniConfig& FindConfig() const;
		const std::string& FindName() const;
		std::string FindQualifiedName() const;
		int32_t FindServerId() const;
		const std::string& FindEnvironment() const;
		int32_t FindWorldId() const;
		int32_t FindTimeoutMs() const;
		const IpEndPoint& FindBindAddress() const;
		const IpEndPoint& FindAdvertiseAddress() const;
		const IpEndPoint& FindDumpAddress() const;
		bool IsLocal() const;

		void SetBindAddress(const std::string& addr);
		void SetAdvertiseAddress(const std::string& addr);
		void SetTimeoutMs(int32_t ms);

	private:
		static std::string FindConfigDir();
		static std::string FindServiceNameFromExe();
	};
}