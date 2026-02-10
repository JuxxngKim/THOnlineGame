#pragma once

namespace th
{
	namespace FileSystem
	{
		std::string GetExecutableFileName();
		std::string GetExecutableDirName();
		std::string GetExecutableFullPath();
		bool Exist(const std::string& path);

		std::vector<std::string> CommandLineToArgv();
	};
}
