#pragma once

namespace th
{
    class ILogChannel
    {
    public:
        virtual ~ILogChannel() = default;
        virtual void Write(const std::string& formattedLine, const LogMessage& rawMsg) = 0;
        virtual void Flush() = 0;
    };
}