#pragma once

namespace th
{
	class IpEndPoint
	{
	private:
		std::string m_address;
		int32_t m_port;

	public:
		IpEndPoint();
		explicit IpEndPoint(const std::string& formatedString); // host_name:port

		std::string Hostname() const;
		int32_t Port() const;

	private:
		static std::string ExtractAddress(const std::string& formatedString);
		static int32_t ExtractPort(const std::string& formatedString);
	};
}
