#pragma once

namespace th
{
	// key:value key:value ...
	class ConnectionString
	{
	private:
		std::map<std::string, std::string> m_values;

	public:
		explicit ConnectionString(const std::string& str);

		bool Valid() const;

		std::string Uid() const;
		std::string Pwd() const;
		std::string Database() const;
		std::string Host() const;
		std::string Port() const;
		int LbRate() const;

	private:
		std::string GetValue(const std::string& key) const;
		static std::vector<std::string> SplitKeyValues(const std::string& str);
		static std::pair<std::string, std::string> SplitKeyValue(const std::string& str);
	};
}
