#include "CommonPch.h"
#include "Logger.h"

namespace th
{
    Logger& Logger::GetInstance()
    {
        static Logger instance;
        return instance;
    }

    Logger::Logger()
        : m_minLevel(ELogLevel::Trace)
        , m_running(false)
    {
    }

    Logger::~Logger()
    {
        Stop();
    }

    void Logger::AddChannel(std::unique_ptr<ILogChannel> channel)
    {
        m_channels.push_back(std::move(channel));
    }

    void Logger::SetMinLevel(ELogLevel level)
    {
        m_minLevel = level;
    }

    ELogLevel Logger::GetMinLevel() const
    {
        return m_minLevel;
    }

    void Logger::Start()
    {
        if (m_running) return;
        m_running = true;

        m_thread = std::make_unique<std::thread>([this] { WorkerLoop(); });
    }

    void Logger::Stop()
    {
        if (!m_running) return;
        m_running = false;

        if (m_thread && m_thread->joinable())
            m_thread->join();

        // 잔여 메시지 처리
        auto& remaining = m_queue.Pop();
        if (!remaining.empty())
        {
            ProcessMessages(remaining);
            remaining.clear();
        }
        FlushAll();
	}

	void Logger::WorkerLoop()
	{
		while (m_running)
		{
			auto& messageBox = m_queue.Pop();
			if (messageBox.empty()) continue;

			ProcessMessages(messageBox);
			messageBox.clear();
		}
	}

    void Logger::ProcessMessages(std::deque<LogMessage>& messages)
    {
        for (const auto& msg : messages)
        {
            const auto line = FormatLogLine(msg);
            for (auto& channel : m_channels)
            {
                channel->Write(line, msg);
            }
        }
    }

    void Logger::FlushAll()
    {
        for (auto& channel : m_channels)
        {
            channel->Flush();
        }
    }
}