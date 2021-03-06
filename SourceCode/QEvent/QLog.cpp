#include "QLog.h"
#include <cstdio>
#include <chrono>
#include <cstring>


QLog g_Log;

std::string getCurrentSystemTime()
{
    auto Now = std::chrono::system_clock::now();

    uint64_t MillisecondsCount = std::chrono::duration_cast<std::chrono::milliseconds>(Now.time_since_epoch()).count();
    uint64_t SecondsCount = std::chrono::duration_cast<std::chrono::seconds>(Now.time_since_epoch()).count();
    uint64_t OffsetCount = MillisecondsCount - SecondsCount * 1000;

    time_t tt = std::chrono::system_clock::to_time_t(Now);
    tm *mytm = localtime(&tt);

    char DateTime[25] = { 0 };
    sprintf(DateTime, "%d-%02d-%02d %02d:%02d:%02d.%03d",
        mytm->tm_year + 1900,
        mytm->tm_mon + 1,
        mytm->tm_mday,
        mytm->tm_hour,
        mytm->tm_min,
        mytm->tm_sec,
        (int)OffsetCount);

    return std::string(DateTime);
}

QLog::QLog()
{
    m_LogFile = NULL;
    m_IsOutputConsole = true;
    m_EnableLogLevel = LL_DEBUG;
}

QLog::~QLog()
{
    if (m_LogFile != NULL)
    {
        fclose(m_LogFile);
        m_LogFile = NULL;
    }
}

void QLog::SetEnableLogLevel(LogLevel Level)
{
    m_EnableLogLevel = Level;
}

bool QLog::SetLogFile(const std::string &FileName)
{
    m_LogFile = fopen(FileName.c_str(), "w");
    if (m_LogFile != NULL)
    {
        setbuf(m_LogFile, NULL);
        return true;
    }

    printf("!!!!!Can not open file = %s\n", FileName.c_str());
    printf("!!!!!errno = %d, description = %s\n", errno, strerror(errno));
    return false;
}

void QLog::SetIsOutputConsole(bool IsOutputConsole)
{
    m_IsOutputConsole = IsOutputConsole;
}

void QLog::WriteDebug(const char *fmt, ...)
{
    va_list ArgList;

    va_start(ArgList, fmt);
    WriteLog(LL_DEBUG, fmt, ArgList);
    va_end(ArgList);
}

void QLog::WriteInfo(const char *fmt, ...)
{
    va_list ArgList;

    va_start(ArgList, fmt);
    WriteLog(LL_INFO, fmt, ArgList);
    va_end(ArgList);
}

void QLog::WriteWarn(const char *fmt, ...)
{
    va_list ArgList;

    va_start(ArgList, fmt);
    WriteLog(LL_WARN, fmt, ArgList);
    va_end(ArgList);
}

void QLog::WriteError(const char *fmt, ...)
{
    va_list ArgList;

    va_start(ArgList, fmt);
    WriteLog(LL_ERROR, fmt, ArgList);
    va_end(ArgList);
}

void QLog::WriteLog(LogLevel Level, const char *fmt, va_list ArgList)
{
    if (Level < m_EnableLogLevel || fmt == NULL)
    {
        return;
    }

    char LogBuffer[1024];
    vsnprintf(LogBuffer, sizeof(LogBuffer), fmt, ArgList);

    const char *LevelString;
    switch(Level)
    {
    case LL_DEBUG:      LevelString = "DEBUG";      break;
    case LL_INFO:       LevelString = "INFO ";      break;
    case LL_WARN:       LevelString = "WARN ";      break;
    case LL_ERROR:      LevelString = "ERROR";      break;
    default:            LevelString = "?????";      break;
    }

    const char *LogFmt = "[%s][%s]: %s\n";
    const std::string &DateTime = getCurrentSystemTime();

    if (m_IsOutputConsole)
    {
        fprintf(stderr, LogFmt, DateTime.c_str(), LevelString, LogBuffer);
    }

    if (m_LogFile != NULL)
    {
        fprintf(m_LogFile, LogFmt, DateTime.c_str(), LevelString, LogBuffer);
    }
}
