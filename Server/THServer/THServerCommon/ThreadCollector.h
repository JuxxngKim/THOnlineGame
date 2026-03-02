#pragma once

namespace util
{
	class ThreadCollector
	{
	public:
		static std::map<std::string, std::vector<int>> s_ServerThreads;
		static std::mutex s_ServerThreadsLock;
		static void InsertThreadInfo(std::string& name, int tid);
		static void PrintThreadInfo();
	};
}