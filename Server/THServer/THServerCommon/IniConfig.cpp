#include "CommonPch.h"
#include "IniConfig.h"

namespace th
{
	bool IniConfig::Load(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open()) return false;

		std::string line;
		std::string currentSection;

		while (std::getline(file, line))
		{
			line = Trim(line);
			if (line.empty() || line[0] == ';' || line[0] == '#')
				continue;

			// [Section]
			if (line.front() == '[' && line.back() == ']')
			{
				currentSection = ToLower(Trim(line.substr(1, line.size() - 2)));
				continue;
			}

			// Key = Value
			auto pos = line.find('=');
			if (pos == std::string::npos || currentSection.empty())
				continue;

			auto key = ToLower(Trim(line.substr(0, pos)));
			auto value = Trim(line.substr(pos + 1));

			m_data[currentSection][key] = value;
		}

		return true;
	}

	std::string IniConfig::Find(const std::string& section, const std::string& key, const std::string& defaultValue) const
	{
		auto sit = m_data.find(ToLower(section));
		if (sit == m_data.end()) return defaultValue;

		auto kit = sit->second.find(ToLower(key));
		if (kit == sit->second.end()) return defaultValue;

		return kit->second;
	}

	int32_t IniConfig::FindInt(const std::string& section, const std::string& key, int32_t defaultValue) const
	{
		auto str = Find(section, key);
		if (str.empty()) return defaultValue;

		try { return std::stoi(str); }
		catch (...) { return defaultValue; }
	}

	bool IniConfig::FindBool(const std::string& section, const std::string& key, bool defaultValue) const
	{
		auto str = ToLower(Find(section, key));
		if (str.empty()) return defaultValue;
		return str == "1" || str == "true";
	}

	bool IniConfig::Has(const std::string& section, const std::string& key) const
	{
		auto sit = m_data.find(ToLower(section));
		if (sit == m_data.end()) return false;
		return sit->second.contains(ToLower(key));
	}

	void IniConfig::Set(const std::string& section, const std::string& key, const std::string& value)
	{
		m_data[ToLower(section)][ToLower(key)] = value;
	}

	std::string IniConfig::Trim(const std::string& str)
	{
		auto s = str.find_first_not_of(" \t\r\n");
		if (s == std::string::npos) return {};
		auto e = str.find_last_not_of(" \t\r\n");
		return str.substr(s, e - s + 1);
	}

	std::string IniConfig::ToLower(const std::string& str)
	{
		std::string result = str;
		std::ranges::transform(result, result.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return result;
	}
}