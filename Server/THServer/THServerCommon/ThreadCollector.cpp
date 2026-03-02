#include "CommonPch.h"
#include "ThreadCollector.h"

namespace util
{
	std::map<std::string, std::vector<int>> ThreadCollector::s_ServerThreads;
	std::mutex ThreadCollector::s_ServerThreadsLock;

	void ThreadCollector::InsertThreadInfo(std::string& name, int tid)
	{
		std::scoped_lock lock(s_ServerThreadsLock);

		auto iter = s_ServerThreads.find(name);
		if (iter == s_ServerThreads.end())
		{
			std::vector<int> tids;
			tids.push_back(tid);
			s_ServerThreads.insert({ name, tids });
		}
		else
		{
			iter->second.push_back(tid);
		}
	}

	void ThreadCollector::PrintThreadInfo()
	{
		std::scoped_lock lock(s_ServerThreadsLock);

		for (auto iter = s_ServerThreads.begin(); iter != s_ServerThreads.end(); ++iter)
		{
			for (auto secIter = iter->second.begin(); secIter != iter->second.end(); ++secIter)
			{
				TH_LOG_DEBUG(0, 0, "ThreadName:%, ThreadID:%", iter->first, *secIter);
			}
		}
	}
}