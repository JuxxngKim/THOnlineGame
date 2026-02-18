#pragma once
#include "ILogChannel.h"

namespace th
{
	// 외부 시스템 연동 채널 (Grafana Loki, Heimdall 등)
	// 현재는 스텁이며, FlushBatch()에서 HTTP POST 구현
	class ExternalChannel : public ILogChannel
	{
	public:
		explicit ExternalChannel(std::string endpoint);

		void SetBatchSize(size_t size);
		void Write(const std::string& formattedLine, const LogMessage& rawMsg) override;
		void Flush() override;

	private:
		void FlushBatch();

		std::string m_endpoint;
		size_t m_batchSize;
		std::vector<std::string> m_buffer;
		std::mutex m_mutex;
	};
}