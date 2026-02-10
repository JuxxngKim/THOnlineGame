#pragma once

namespace th
{
	namespace IniParser
	{
		std::vector<std::string> ReadSectionNames(const std::string& filePath);
		std::vector<std::string> ReadSection(const std::string& name, const std::string& filePath);
		std::vector<std::string> SplitNullDelimitedString(const char* str);
		std::pair<std::string, std::string> SplitSectionString(const std::string& str);		
	}
}
