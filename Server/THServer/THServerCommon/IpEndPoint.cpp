#include "CommonPch.h"
#include "IpEndPoint.h"

namespace th
{
	IpEndPoint::IpEndPoint()
		: m_address("127.0.0.1")
		, m_port(35501)
	{
	}

	IpEndPoint::IpEndPoint(const std::string& str)
		: m_address("127.0.0.1")
		, m_port(35501)
	{
		if (str.empty()) return;
		if (str == "localhost") return;

		auto pos = str.find(':');
		if (pos != std::string::npos)
		{
			m_address = str.substr(0, pos);
			try { m_port = std::stoi(str.substr(pos + 1)); }
			catch (...) { m_port = 0; }
		}

		if (m_address.empty())
		{
			m_address = "127.0.0.1";
			m_port = 35501;
		}
	}

	const std::string& IpEndPoint::FindHost() const
	{
		return m_address;
	}

	int32_t IpEndPoint::FindPort() const
	{
		return m_port;
	}

	std::string IpEndPoint::ToString() const
	{
		return m_address + ":" + std::to_string(m_port);
	}
}