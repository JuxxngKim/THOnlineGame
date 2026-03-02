#include "CommonPch.h"
#include "ConfigValue.h"
#include "IpEndPoint.h"
#include "ConnectionString.h"
#include "StrUtil.h"

namespace th
{
	ConfigValue::ConfigValue(const std::string& value)
		: m_value(value)
	{}

	ConfigValue ConfigValue::Default(const std::string& def)
	{
		if (m_value.empty())
		{
			return{ def };
		}

		return *this;
	}

	ConfigValue ConfigValue::Default(int def)
	{
		if (m_value.empty())
		{
			return{ std::to_string(def) };
		}

		return *this;
	}

	ConfigValue ConfigValue::Default(bool def)
	{
		if (m_value.empty())
		{
			return{ def ? "true" : "false" };
		}

		return *this;
	}

	bool ConfigValue::Valid() const
	{
		return !m_value.empty();
	}

	int ConfigValue::ToInt() const
	{
		return StrUtil::ToInt32(m_value);
	}

	std::string ConfigValue::ToLower() const
	{
		return StrUtil::ToLower(m_value);
	}

	IpEndPoint ConfigValue::ToIpEndPoint() const
	{
		return IpEndPoint(m_value);
	}

	ConnectionString ConfigValue::ToConnectionString() const
	{
		return ConnectionString(m_value);
	}

	ConfigValue::operator std::string() const
	{
		return m_value;
	}

	ConfigValue::operator int() const
	{
		return StrUtil::ToInt32(m_value);
	}

	ConfigValue::operator size_t() const
	{
		return StrUtil::ToUnsignedLong(m_value);
	}

	ConfigValue::operator bool() const
	{
		auto str = StrUtil::ToLower(m_value);

		if (str == "1") return true;
		if (str == "true") return true;

		return false;
	}
}
