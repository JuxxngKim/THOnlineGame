#pragma once

namespace th
{
	class IniSection
	{
	private:
		std::map<std::string, std::string> m_values;

	public:
		bool Load(const std::string& sectionName, const std::string& filePath);
		std::string Get(const std::string& key) const;
		void Set(const std::string& key, std::string value);

	private:
		bool LoadKeyValues(const std::vector<std::string>& sections);
	};
}
