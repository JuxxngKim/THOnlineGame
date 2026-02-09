#include "GamePch.h"
#include <iostream>

int32_t main(int arge, const char** argv)
{
	std::cout << "hello world!" << '\n';

	const auto& time = util::TimeUtil::GetInstance().FindKSTDate();
	std::cout << time.Day << ":" << time.Hour << ":" << time.Minute << '\n';

	return 0;
}
