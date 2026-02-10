#include "CommonPCH.h"
#include "FileSystem.h"
#include <fstream>
#include <shellapi.h>
#include "StrUtil.h"

namespace th
{
	namespace FileSystem
	{
		std::string GetExecutableFileName()
		{
			auto fullPath = GetExecutableFullPath();
			char filename[_MAX_FNAME];
			char ext[_MAX_EXT];
			_splitpath_s(fullPath.c_str(), nullptr, 0, nullptr, 0, filename, _MAX_FNAME, ext, _MAX_EXT);
			return std::string(filename);
		}

		std::string GetExecutableDirName()
		{
			auto fullPath = GetExecutableFullPath();
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			_splitpath_s(fullPath.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR, nullptr, 0, nullptr, 0);
			return std::string(drive) + std::string(dir);
		}

		std::string GetExecutableFullPath()
		{
			// Windows: MAX_PATH 제한 우회를 위해 충분한 버퍼 사용
			char buffer[MAX_PATH]{};
			::GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
			return buffer;
		}

		bool Exist(const std::string& path)
		{
			std::ifstream f(path);
			return f.good();
		}

		std::vector<std::string> CommandLineToArgv()
		{
			std::vector<std::string> result;

			int argc{};
			auto args = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

			result.reserve(argc);
			for (int i = 0; i < argc; i++)
			{
				result.push_back(StrUtil::ToUTF8(args[i]));
			}

			return result;
		}
	}
}
