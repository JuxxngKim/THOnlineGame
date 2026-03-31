#include "CommonPch.h"
#include "Configuration.h"
#include "IniSection.h"
//#include "FileSystem.h"
#include "ConfigSource.h"

namespace th
{
	Configuration::Configuration()
	{
		// Runtime mutable ini
		m_inis.emplace_back();
	}

	Configuration::~Configuration()
	{}

	bool Configuration::Load(const std::vector<std::string>& profileFullPaths)
	{
		// Initial config load
		std::string env{ Get("Profile", "Env").Default(std::string{"local"}) };
		int id{ Get("Profile", "Id").Default(1) };
		auto section = ConfigSource::MakeServiceName();
		std::string service = Get("Profile", "Service").Default(section);

		if (LoadInis(profileFullPaths))
		{
			env = Get("Profile", "Env");
			id = Get("Profile", "Id");
		}

		// Load config
		return Load(env, service, id);
	}

	bool Configuration::Load(const std::string& env, const std::string& service, int id)
	{
		auto paths = ConfigSource::MakeConfigFullPaths(env);
		if (LoadInis(paths))
		{
			SetProfile(env, service, id);
			TH_LOG_INFO(0, 0, "Env:% - Service:% - ID:%", env.c_str(), service.c_str(), id);
			return true;
		}

		return false;
	}

	bool Configuration::LoadInis(const std::vector<std::string>& fullPaths)
	{
		for (auto& it : fullPaths)
		{
			IniFile ini;
			if (!ini.Load(it)) return false;

			m_inis.push_back(ini);
		}

		return true;
	}

	void Configuration::SetProfile(const std::string& env, const std::string& service, int id)
	{
		Set("Profile", "Env", env);
		Set("Profile", "Service", service);
		Set("Profile", "Id", id);
	}

	ConfigValue Configuration::Get(const std::string& section, const std::string& key) const
	{
		for (auto& ini : m_inis)
		{
			auto value = ini.Get(section).Get(key);
			if (!value.empty())
			{
				return{ value };
			}
		}

		return{};
	}

	void Configuration::Set(const std::string& section, const std::string& key, std::string value)
	{
		auto& mutableIni = m_inis.front();
		mutableIni.Mutable(section).Set(key, value);
	}

	void Configuration::Set(const std::string& section, const std::string& key, int value)
	{
		auto& mutableIni = m_inis.front();
		mutableIni.Mutable(section).Set(key, std::to_string(value));
	}
}
