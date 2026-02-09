#include "GamePch.h"
#include <iostream>
#include "ServiceProfile.h"

int32_t main(int arge, const char** argv)
{
	th::ServiceProfile::GetInstance().Load();
	std::cout << "TimeoutMs:" << th::ServiceProfile::GetInstance().FindTimeoutMs() << '\n';

	return 0;
}