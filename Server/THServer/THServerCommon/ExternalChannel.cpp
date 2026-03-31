#include "CommonPch.h"
#include "ExternalChannel.h"

namespace th
{
	ExternalChannel::ExternalChannel(std::string endpoint)
		: m_endpoint(std::move(endpoint))
		, m_batchSize(50)
	{
	}

	void ExternalChannel::SetBatchSize(size_t size)
	{
		m_batchSize = size;
	}

	void ExternalChannel::Write(const std::string& formattedLine, const LogMessage& /*rawMsg*/)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_buffer.push_back(formattedLine);

		if (m_buffer.size() >= m_batchSize)
		{
			FlushBatch();
		}
	}

	void ExternalChannel::Flush()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_buffer.empty())
			FlushBatch();
	}

	void ExternalChannel::FlushBatch()
	{
		// Loki: { "streams": [{ "stream": { "job": "th_server" },
		//   "values": [["timestamp_ns", "log line"], ...] }] }
		m_buffer.clear();
	}
}