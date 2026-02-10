#include "CommonPCH.h"
#include "IniFile.h"
#include "IniSection.h"
#include "IniParser.h"
#include "StrUtil.h"

namespace th
{
	bool IniFile::Load(const std::string& filePath)
	{
		auto names = IniParser::ReadSectionNames(filePath);
		return LoadSections(names, filePath);
	}

	const IniSection& IniFile::Get(const std::string& section) const
	{
		auto it = m_sections.find(StrUtil::ToLower(section));
		if (it == m_sections.end())
		{
			static IniSection empty;
			return empty;
		}

		return it->second;
	}

	IniSection& IniFile::Mutable(const std::string& section)
	{
		return m_sections[StrUtil::ToLower(section)];
	}

	bool IniFile::LoadSections(const std::vector<std::string>& sectionNames, const std::string& filePath)
	{
		for (auto& name : sectionNames)
		{
			IniSection section;
			if (!section.Load(name, filePath)) return false;

			m_sections.insert({ StrUtil::ToLower(name), section });
		}

		return !m_sections.empty();
	}
}
