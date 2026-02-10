#pragma once

namespace th
{
	class IpEndPoint;
	class ConnectionString;

	class ConfigValue
	{
	private:
		std::string m_value;

	public:
		ConfigValue() = default;
		ConfigValue(const std::string& value);

		ConfigValue Default(const std::string& def);
		ConfigValue Default(int def);
		ConfigValue Default(bool def);

		bool Valid() const;

		int ToInt() const;
		std::string ToLower() const;
		IpEndPoint ToIpEndPoint() const;
		ConnectionString ToConnectionString() const;

		template <typename T>
		T ToEnum() const
		{
			return static_cast<T>(ToInt());
		}

		operator std::string() const;
		operator int() const;
		operator size_t() const;
		operator bool() const;
	};
}
