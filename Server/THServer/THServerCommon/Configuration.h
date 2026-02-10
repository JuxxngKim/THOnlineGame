#pragma once

#include "IniFile.h"
#include "ConfigValue.h"

namespace th
{
	class Configuration : public Singleton<Configuration>
	{
	private:
		std::vector<IniFile> m_inis;

	public:
		Configuration();
		virtual ~Configuration();

		bool Load(const std::vector<std::string>& profileFullPath);
		bool Load(const std::string& env, const std::string& service, int id);

		ConfigValue Get(const std::string& section, const std::string& key) const;
		void Set(const std::string& section, const std::string& key, std::string value);
		void Set(const std::string& section, const std::string& key, int value);

	private:
		bool LoadInis(const std::vector<std::string>& profileFullPaths);
		void SetProfile(const std::string& env, const std::string& service, int id);
	};
}
