#include "CommonPch.h"
#include "LogFormatter.h"

namespace th
{
	std::string FormatTimestamp(const THDateTime& dt)
	{
		char buf[64];
		std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
			dt.Year, dt.Month, dt.Day,
			dt.Hour, dt.Minute, dt.Second,
			dt.Milliseconds);
		return buf;
	}

	std::string FormatDateFolder(const THDateTime& dt)
	{
		char buf[32];
		std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
			dt.Year, dt.Month, dt.Day);
		return buf;
	}

	std::string FormatTimeForFilename(const THDateTime& dt)
	{
		char buf[16];
		std::snprintf(buf, sizeof(buf), "%02d%02d%02d",
			dt.Hour, dt.Minute, dt.Second);
		return buf;
	}

	std::string FormatLogLine(const LogMessage& msg)
	{
		// [2026-02-11 10:29:50.020][E]hostID:724 accountUID:2181 [Alarm] message text
		std::string line;
		line.reserve(256);
		line += '[';
		line += FormatTimestamp(msg.Timestamp);
		line += "][";
		line += ToShortString(msg.Level);
		line += ']';
		line += msg.Text;
		return line;
	}

	const char* ToShortString(ELogLevel level)
	{
		switch (level)
		{
		case ELogLevel::Trace: return "T";
		case ELogLevel::Debug: return "D";
		case ELogLevel::Info: return "I";
		case ELogLevel::Warn: return "W";
		case ELogLevel::Error: return "E";
		case ELogLevel::Critical: return "C";
		default: return "?";
		}
	}

	const char* ToString(ELogLevel level)
	{
		switch (level)
		{
		case ELogLevel::Trace: return "TRACE";
		case ELogLevel::Debug: return "DEBUG";
		case ELogLevel::Info: return "INFO";
		case ELogLevel::Warn: return "WARN";
		case ELogLevel::Error: return "ERROR";
		case ELogLevel::Critical: return "CRITICAL";
		default: return "UNKNOWN";
		}
	}
}