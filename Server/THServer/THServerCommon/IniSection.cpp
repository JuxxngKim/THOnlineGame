#include "CommonPCH.h"
#include "IniSection.h"
#include "IniParser.h"
#include "StrUtil.h"

namespace th
{
	bool IniSection::Load(const std::string& sectionName, const std::string& filePath)
	{
		auto sections = IniParser::ReadSection(sectionName, filePath);
		return LoadKeyValues(sections);
	}

	std::string IniSection::Get(const std::string& key) const
	{
		auto it = m_values.find(StrUtil::ToLower(key));
		if (it == m_values.end()) return{};

		return it->second;
	}

	void IniSection::Set(const std::string& key, std::string value)
	{
		if (value.empty()) return;		
		m_values[StrUtil::ToLower(key)] = value;
	}

	bool IniSection::LoadKeyValues(const std::vector<std::string>& sections)
	{
		for (auto& section : sections)
		{
			auto keyValue = IniParser::SplitSectionString(section);
			if (keyValue.first.empty()) return false;

			m_values.insert({ StrUtil::ToLower(keyValue.first), keyValue.second });
		}

		return !m_values.empty();
	}
}
