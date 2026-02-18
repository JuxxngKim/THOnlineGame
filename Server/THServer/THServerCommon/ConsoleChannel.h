#pragma once
#include "ILogChannel.h"

namespace th
{
	class ConsoleChannel : public ILogChannel
	{
	public:
		void Write(const std::string& formattedLine, const LogMessage& rawMsg) override;
		void Flush() override;

	private:
		std::mutex m_mutex;
	};
}