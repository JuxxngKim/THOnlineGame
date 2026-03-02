#include "CommonPch.h"
#include "IpEndPoint.h"
#include "StrUtil.h"

namespace th
{
	IpEndPoint::IpEndPoint()
		: m_port(0)
	{}

	IpEndPoint::IpEndPoint(const std::string& formatedString)
	{
		m_address = ExtractAddress(formatedString);
		m_port = ExtractPort(formatedString);
	}

	std::string IpEndPoint::Hostname() const
	{
		return m_address;
	}

	int32_t IpEndPoint::Port() const
	{
		return m_port;
	}

	std::string IpEndPoint::ExtractAddress(const std::string& formatedString)
	{
		// NOTE: "localhost"는 정책변경 시에도 영향받지 않도록 별도처리한다.
		if (formatedString == "localhost") return "127.0.0.1";

		auto pos = formatedString.find(':');
		auto address = formatedString.substr(0, pos);
		if (address.empty())	return "127.0.0.1";

		return address;
	}

	int32_t IpEndPoint::ExtractPort(const std::string& formatedString)
	{
		auto defaultPort = 35501;

		auto pos = formatedString.find(':');
		if (pos == std::string::npos) return defaultPort;

		auto port = StrUtil::ToInt32(formatedString.substr(pos + 1));
		if (port == 0) return defaultPort;

		return port;
	}
}
