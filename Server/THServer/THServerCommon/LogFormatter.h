#pragma once
#include <string>
#include <sstream>
#include "THDateTime.h"

namespace th
{
    // --- % placeholder formatting (template - header 유지) ---

    inline void FormatImpl(std::ostringstream& oss, const char* fmt)
    {
        oss << fmt;
    }

    template<typename T, typename... Args>
    void FormatImpl(std::ostringstream& oss, const char* fmt, T&& value, Args&&... args)
    {
        while (*fmt)
        {
            if (*fmt == '%')
            {
                oss << std::forward<T>(value);
                FormatImpl(oss, fmt + 1, std::forward<Args>(args)...);
                return;
            }
            oss << *fmt++;
        }
    }

    template<typename... Args>
    std::string Format(const char* fmt, Args&&... args)
    {
        std::ostringstream oss;
        FormatImpl(oss, fmt, std::forward<Args>(args)...);
        return oss.str();
    }

    // --- THDateTime 기반 포맷 함수 (cpp에서 구현) ---

    // "2026-02-11 10:29:50.020"
    std::string FormatTimestamp(const THDateTime& dt);

    // "2026-02-11" (폴더명용)
    std::string FormatDateFolder(const THDateTime& dt);

    // "102950" (파일명용)
    std::string FormatTimeForFilename(const THDateTime& dt);

    // "[2026-02-11 10:29:50.020][E] message text"
    std::string FormatLogLine(const LogMessage& msg);

    const char* ToShortString(ELogLevel level);

    const char* ToString(ELogLevel level);
}