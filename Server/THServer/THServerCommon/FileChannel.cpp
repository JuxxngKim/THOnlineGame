#include "CommonPch.h"
#include "FileChannel.h"
#include "LogFormatter.h"

namespace th
{
    FileChannel::FileChannel(const std::string& baseDir, const std::string& prefix)
        : m_baseDir(baseDir)
        , m_prefix(prefix)
    {
        const auto startTime = util::TimeUtil::GetInstance().FindKSTDate();
        m_currentDate = FormatDateFolder(startTime);
        OpenNewFile(startTime);
    }

    FileChannel::~FileChannel()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open())
        {
            m_file.flush();
            m_file.close();
        }
    }

    void FileChannel::Write(const std::string& formattedLine, const LogMessage& rawMsg)
    {
	    std::scoped_lock lock(m_mutex);

        const auto msgDate = FormatDateFolder(rawMsg.Timestamp);
        if (msgDate != m_currentDate)
        {
            m_currentDate = msgDate;
            OpenNewFile(rawMsg.Timestamp);
        }

        if (m_file.is_open())
        {
            m_file << formattedLine << '\n';

            ++m_writeCount;
            if (m_writeCount >= kFlushInterval)
            {
                m_file.flush();
                m_writeCount = 0;
            }
        }
    }

    void FileChannel::Flush()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file.is_open())
            m_file.flush();
    }

    void FileChannel::OpenNewFile(const THDateTime& dt)
    {
        if (m_file.is_open())
        {
            m_file.flush();
            m_file.close();
        }

        const auto dateStr = FormatDateFolder(dt);
        const auto timeStr = FormatTimeForFilename(dt);

        auto dirPath = std::filesystem::path(m_baseDir) / dateStr;
        std::filesystem::create_directories(dirPath);

        auto filePath = dirPath / (m_prefix + "_" + timeStr + ".log");
        m_file.open(filePath.string(), std::ios::out | std::ios::app);
    }
}