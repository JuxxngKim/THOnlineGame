#pragma once
#include "ILogChannel.h"
#include "THDateTime.h"

namespace th
{
	class FileChannel : public ILogChannel
	{
	public:
		FileChannel(const std::string& baseDir, const std::string& prefix);
		~FileChannel() override;

		void Write(const std::string& formattedLine, const LogMessage& rawMsg) override;
		void Flush() override;

	private:
		void OpenNewFile(const THDateTime& dt);

		static constexpr int kFlushInterval = 50;

		std::string m_baseDir;
		std::string m_prefix;
		std::string m_currentDate;
		std::ofstream m_file;
		std::mutex m_mutex;
		int m_writeCount = 0;
	};
}