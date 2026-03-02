#include "CommonPch.h"
#include "IniParser.h"

namespace th
{
	namespace IniParser
	{
		std::vector<std::string> ReadSectionNames(const std::string& filePath)
		{
			if (filePath.empty())
			{
				// TODO(KJY): Logger
				//S_LOG_ERROR(0,0,"memo:filePath_is_empty");
				return{};
			}

			char buffer[4096] = {};
			auto len = ::GetPrivateProfileSectionNamesA(buffer, sizeof(buffer), filePath.c_str());
			if (len == sizeof(buffer) - 2)
			{
				// TODO(KJY): Logger
				//S_LOG_ERROR(0, 0, "%s memo:check buffer size", filePath);
				return{};
			}

			if (NO_ERROR == ::GetLastError())
			{
				return SplitNullDelimitedString(buffer);
			}

			return{};
		}

		std::vector<std::string> ReadSection(const std::string& name, const std::string& filePath)
		{
			if (filePath.empty())
			{
				// TODO(KJY): Logger
				//S_LOG_ERROR(0, 0, "memo:filePath_is_empty");
				return{};
			}

			char buffer[4096] = {};
			auto len = ::GetPrivateProfileSectionA(name.c_str(), buffer, sizeof(buffer), filePath.c_str());
			if (len == sizeof(buffer) - 2)
			{
				// TODO(KJY): Logger
				//S_LOG_ERROR(0, 0, "%s memo:check buffer size", filePath);
				return{};
			}

			if (NO_ERROR == ::GetLastError())
			{
				return SplitNullDelimitedString(buffer);
			}

			return{};
		}

		std::vector<std::string> SplitNullDelimitedString(const char* str)
		{
			std::vector<std::string> result;

			while (*str != '\0')
			{
				result.emplace_back(str);
				str += result.back().size() + 1;
			}

			return result;
		}

		std::pair<std::string, std::string> SplitSectionString(const std::string& str)
		{
			auto pos = str.find('=');
			if (pos == std::string::npos) return{};
			if (pos == 0) return{};

			return{ str.substr(0, pos), str.substr(pos + 1) };
		}
	}
}
