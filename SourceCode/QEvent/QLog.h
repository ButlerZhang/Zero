#pragma once
#include <stdarg.h>
#include <string>



enum LogLevel
{
    LL_DEBUG = 0,
    LL_INFO,
    LL_WARN,
    LL_ERROR
};

class QLog
{
public:

    QLog();
    ~QLog();

    void SetEnableLogLevel(LogLevel Level);
    bool SetLogFile(const std::string &FileName);
    void SetIsOutputConsole(bool IsOutputConsole);

    void WriteDebug(const char *fmt, ...);
    void WriteInfo(const char *fmt, ...);
    void WriteWarn(const char *fmt, ...);
    void WriteError(const char *fmt, ...);

private:

    void WriteLog(LogLevel Level, const char *fmt, va_list ArgList);

private:

    bool            m_IsOutputConsole;
    LogLevel        m_EnableLogLevel;
    FILE            *m_LogFile;
};

extern QLog g_Log;
