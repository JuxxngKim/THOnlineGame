#pragma once

namespace th
{
	namespace ConfigSource
	{
		std::vector<std::string> MakeInitialConfigFullPaths();
		std::vector<std::string> MakeConfigFullPaths(const std::string& env);
		std::vector<std::string> MakeFullPaths(const std::string& filename);
		std::string MakeServiceName();

		std::string GetConfigDirName();
	}
}
