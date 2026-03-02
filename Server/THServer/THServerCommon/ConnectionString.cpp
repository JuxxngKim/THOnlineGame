#include "CommonPch.h"
#include "ConnectionString.h"
#include "StrUtil.h"

namespace th
{
	ConnectionString::ConnectionString(const std::string& str)
	{
		auto items = SplitKeyValues(str);
		for (auto& item : items)
		{
			auto keyValue = SplitKeyValue(item);
			if (keyValue.first.empty()) continue;

			m_values.insert({ StrUtil::ToLower(keyValue.first), keyValue.second });
		}
	}

	std::vector<std::string> ConnectionString::SplitKeyValues(const std::string& str)
	{
		std::vector<std::string> result;		
		
		std::string keyValue;
		std::istringstream stream(str);
		while (std::getline(stream, keyValue, ' '))
		{
			result.push_back(keyValue);
		}

		return result;
	}

	std::pair<std::string, std::string> ConnectionString::SplitKeyValue(const std::string& str)
	{
		auto pos = str.find(':');
		if (pos == std::string::npos) return{};
		if (pos == 0) return{};

		return{ str.substr(0, pos), str.substr(pos + 1) };
	}

	bool ConnectionString::Valid() const
	{
		return !m_values.empty();
	}

	std::string ConnectionString::Uid() const
	{
		return GetValue("uid");
	}

	std::string ConnectionString::Pwd() const
	{
		return GetValue("pwd");
	}

	std::string ConnectionString::Database() const
	{
		return GetValue("database");
	}

	std::string ConnectionString::Host() const
	{
		return GetValue("host");
	}

	std::string ConnectionString::Port() const
	{
		return GetValue("port");
	}

	int ConnectionString::LbRate() const
	{
		auto str = GetValue("lbrate");
		return StrUtil::ToInt32(str);
	}

	std::string ConnectionString::GetValue(const std::string& key) const
	{
		auto it = m_values.find(StrUtil::ToLower(key));
		if (it == m_values.end()) return{};

		return it->second;
	}
}
