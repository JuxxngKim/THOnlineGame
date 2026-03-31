#pragma once

namespace th
{
    class LogKeyGenerator
    {
    public:
        static LogKey IssueLogKey(const EMessageID& msgID)
        {
            return (util::TimeUtil::GetInstance().FindKSTMs() * 100000) + msgID;
        }
    };
}