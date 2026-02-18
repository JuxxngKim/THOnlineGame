#include "CommonPch.h"
#include "ConsoleChannel.h"
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace th
{
#ifdef _WIN32
	static WORD GetColor(ELogLevel level)
	{
		switch (level)
		{
		case ELogLevel::Trace: return 8;
		case ELogLevel::Debug: return 7;
		case ELogLevel::Info: return 10;
		case ELogLevel::Warn: return 14;
		case ELogLevel::Error: return 12;
		case ELogLevel::Critical: return 79;
		default: return 7;
		}
	}
#else
	static const char* GetAnsiColor(ELogLevel level)
	{
		switch (level)
		{
		case ELogLevel::Trace: return "\033[90m";
		case ELogLevel::Debug: return "\033[37m";
		case ELogLevel::Info: return "\033[32m";
		case ELogLevel::Warn: return "\033[33m";
		case ELogLevel::Error: return "\033[31m";
		case ELogLevel::Critical: return "\033[41;37m";
		default: return "\033[0m";
		}
	}
#endif

	void ConsoleChannel::Write(const std::string& formattedLine, const LogMessage& rawMsg)
	{
		std::lock_guard<std::mutex> lock(m_mutex);

#ifdef _WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		WORD color = GetColor(rawMsg.Level);
		SetConsoleTextAttribute(hConsole, color);
		std::cout << formattedLine << '\n';
		SetConsoleTextAttribute(hConsole, 7);
#else
		std::cout << GetAnsiColor(rawMsg.Level) << formattedLine << "\033[0m" << '\n';
#endif
	}

	void ConsoleChannel::Flush()
	{
		std::cout.flush();
	}
}