#include "CommonPch.h"
#include "ServiceProfile.h"

namespace th
{
	ServiceProfile::ServiceProfile()
		: m_serverId(0)
		, m_worldId(0)
		, m_timeoutMs(0)
	{
	}

	bool ServiceProfile::Load(const std::string& configDir)
	{
		auto dir = configDir.empty() ? FindConfigDir() : configDir;

		// 1) profile.ini 로드
		if (!m_config.Load(dir + "profile.ini"))
			return false;

		// 2) 환경 결정 → config.{env}.ini 로드
		m_environment = m_config.Find("profile", "env", "local");

		auto envFile = dir + "config." + m_environment + ".ini";
		m_config.Load(envFile);
		
		// 3) 프로필 정보 파싱
		m_name = m_config.Find("profile", "service");
		if (m_name.empty())
			m_name = FindServiceNameFromExe();

		m_serverId = m_config.FindInt("profile", "id", 1);
		m_worldId = m_config.FindInt("world", "id", 0);

		// 4) 서비스별 설정 (예: gateway.1)
		auto section = FindQualifiedName();

		m_bindAddress = IpEndPoint(m_config.Find(section, "bindaddr"));
		m_advertiseAddress = IpEndPoint(m_config.Find(section, "advertiseaddr"));
		m_dumpAddress = IpEndPoint(m_config.Find("dumpserver", "serveraddr"));
		m_timeoutMs = m_config.FindInt(section, "networktimeoutms", 0);

		return true;
	}

	const IniConfig& ServiceProfile::FindConfig() const
	{
		return m_config;
	}

	const std::string& ServiceProfile::FindName() const
	{
		return m_name;
	}

	std::string ServiceProfile::FindQualifiedName() const
	{
		return m_name + "." + std::to_string(m_serverId);
	}

	int32_t ServiceProfile::FindServerId() const
	{
		return m_serverId;
	}

	const std::string& ServiceProfile::FindEnvironment() const
	{
		return m_environment;
	}

	int32_t ServiceProfile::FindWorldId() const
	{
		return m_worldId;
	}

	int32_t ServiceProfile::FindTimeoutMs() const
	{
		return m_timeoutMs;
	}

	const IpEndPoint& ServiceProfile::FindBindAddress() const
	{
		return m_bindAddress;
	}

	const IpEndPoint& ServiceProfile::FindAdvertiseAddress() const
	{
		return m_advertiseAddress;
	}

	const IpEndPoint& ServiceProfile::FindDumpAddress() const
	{
		return m_dumpAddress;
	}

	bool ServiceProfile::IsLocal() const
	{
		return m_environment == "local";
	}

	void ServiceProfile::SetBindAddress(const std::string& addr)
	{
		m_bindAddress = IpEndPoint(addr);
	}

	void ServiceProfile::SetAdvertiseAddress(const std::string& addr)
	{
		m_advertiseAddress = IpEndPoint(addr);
	}

	void ServiceProfile::SetTimeoutMs(int32_t ms)
	{
		m_timeoutMs = ms;
	}

	std::string ServiceProfile::FindConfigDir()
	{
#if defined(_WIN32)
		char buffer[1024]{};
		::GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
		auto dir = std::filesystem::path(buffer).parent_path() / "config";
#elif defined(__linux__)
		auto dir = std::filesystem::canonical("/proc/self/exe").parent_path() / "config";
#else
		auto dir = std::filesystem::current_path() / "config";
#endif
		return dir.string() + std::string(1, std::filesystem::path::preferred_separator);
	}

	std::string ServiceProfile::FindServiceNameFromExe()
	{
		std::string serviceName;

#if defined(_WIN32)
		char buffer[1024]{};
		::GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
		serviceName = std::filesystem::path(buffer).stem().string();
#elif defined(__linux__)
		serviceName = std::filesystem::canonical("/proc/self/exe").stem().string();
#else
		return "unknown";
#endif

		auto pattern = "^(TH)?(Server)?(.*?)(d)?$";
		std::regex r(pattern, std::regex::ECMAScript | std::regex::icase);
		std::smatch m;

		if (std::regex_match(serviceName, m, r))
		{
			return m[3].str();  // "Game", "GateWay"
		}

		return "";
	}
}