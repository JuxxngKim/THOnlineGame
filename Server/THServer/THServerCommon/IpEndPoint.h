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
		explicit IpEndPoint(const std::string& str);

		const std::string& FindHost() const;
		int32_t FindPort() const;
		std::string ToString() const;
	};
}