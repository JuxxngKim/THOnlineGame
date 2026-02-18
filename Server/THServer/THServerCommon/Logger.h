#pragma once
#include "LogFormatter.h"
#include "ILogChannel.h"
#include "DualQueue.h"
#include "TimeUtil.h"

namespace th
{
    class Logger
    {
    public:
        static Logger& GetInstance();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void AddChannel(std::unique_ptr<ILogChannel> channel);
        void SetMinLevel(ELogLevel level);
        ELogLevel GetMinLevel() const;
        void Start();
        void Shutdown();

        template<typename... Args>
        void Log(ELogLevel level, const char* fmt, Args&&... args)
        {
            if (level < m_minLevel.load(std::memory_order_relaxed))
                return;

            LogMessage msg;
            msg.Level = level;
            msg.Timestamp = util::TimeUtil::GetInstance().FindKSTDate();

            if constexpr (sizeof...(args) > 0)
                msg.Text = Format(fmt, std::forward<Args>(args)...);
            else
                msg.Text = fmt;

            m_queue.Push(std::move(msg));
        }

        template<typename... Args>
        void Trace(const char* fmt, Args&&... args)
        {
            Log(ELogLevel::Trace, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Debug(const char* fmt, Args&&... args)
        {
            Log(ELogLevel::Debug, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Info(const char* fmt, Args&&... args)
        {
            Log(ELogLevel::Info, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Warn(const char* fmt, Args&&... args)
        {
            Log(ELogLevel::Warn, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Error(const char* fmt, Args&&... args)
        {
            Log(ELogLevel::Error, fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Critical(const char* fmt, Args&&... args)
        {
            Log(ELogLevel::Critical, fmt, std::forward<Args>(args)...);
        }

    private:
        Logger();
        ~Logger();

        void WorkerLoop();
        void ProcessMessages(std::deque<LogMessage>& messages);
        void FlushAll();

        std::atomic<ELogLevel> m_minLevel;
        std::atomic<bool> m_running;
        util::DualQueue<LogMessage> m_queue;
        std::vector<std::unique_ptr<ILogChannel>> m_channels;
        std::unique_ptr<std::thread> m_thread;
    };
}