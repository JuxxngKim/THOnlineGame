#include "GamePch.h"
#include <iostream>

#include "ConfigSource.h"
#include "Configuration.h"
#include "ServiceProfile.h"

int32_t main(int arge, const char** argv)
{
	auto& config = th::Configuration::GetInstance();
	auto paths = th::ConfigSource::MakeInitialConfigFullPaths();
	if (!config.Load(paths)) return 0;

	auto& profile = th::ServiceProfile::GetInstance();
	profile.Load(config);

	const auto& test = google::protobuf::MessageFactory::generated_factory();

	return 0;
}