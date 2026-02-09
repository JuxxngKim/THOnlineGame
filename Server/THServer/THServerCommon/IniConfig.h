#pragma once

namespace th
{
	class IniConfig
	{
	private:
		using SectionMap = std::map<std::string, std::string>;
		using DataMap = std::map<std::string, SectionMap>;

		DataMap m_data;

		static std::string Trim(const std::string& str);
		static std::string ToLower(const std::string& str);

	public:
		bool Load(const std::string& filePath);

		std::string Find(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;
		int32_t FindInt(const std::string& section, const std::string& key, int32_t defaultValue = 0) const;
		bool FindBool(const std::string& section, const std::string& key, bool defaultValue = false) const;
		bool Has(const std::string& section, const std::string& key) const;
		void Set(const std::string& section, const std::string& key, const std::string& value);
	};
}