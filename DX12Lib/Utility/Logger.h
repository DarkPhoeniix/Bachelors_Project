#pragma once

#define ASSERT(statement, message) \
    AssertUtility::AssertFunction(statement, message)

#define LOG_WARNING(statement, message) \
    AssertUtility::LogWarningFunction(statement, message)

#define LOG_INFO(statement, message) \
    AssertUtility::LogInfoFunction(statement, message)

namespace AssertUtility
{
    bool AssertFunction(bool statement, const std::string& message);
    bool LogWarningFunction(bool statement, const std::string& message);
    bool LogInfoFunction(bool statement, const std::string& message);
}

enum class LogType
{
    Info    = 1,
    Warning = 2,
    Error   = 4
};

LogType operator&(LogType lhs, LogType rhs);
LogType operator|(LogType lhs, LogType rhs);

class Logger
{
public:
    Logger(const Logger& copy) = delete;
    Logger& operator=(const Logger& copy) = delete;

    static Logger& Instance();

    static void Log(LogType type, const std::string& message);
    static void SetLogLevel(LogType logLevel);
    static LogType GetLogLevel();

private:
    Logger();
    ~Logger();

    std::fstream _logFile;

    static LogType _logLevel;
};
