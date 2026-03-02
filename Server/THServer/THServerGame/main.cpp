#include "GamePch.h"
#include "GameServerApp.h"

int32_t main(int arge, const char** argv)
{
	th::GameServerApp app;
	app.Start();

	return 0;
}
