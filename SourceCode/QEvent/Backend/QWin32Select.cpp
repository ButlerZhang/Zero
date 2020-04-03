#include "QWin32Select.h"
#include "../../QLog/QSimpleLog.h"
#include "../Network/QNetwork.h"



QWin32Select::QWin32Select()
{
    m_BackendName = "win32select";
    memset(&m_ReadSetIn, 0, sizeof(m_ReadSetIn));
    memset(&m_WriteSetIn, 0, sizeof(m_WriteSetIn));
}

QWin32Select::~QWin32Select()
{
}

bool QWin32Select::AddEvent(const QEvent &Event)
{
    if (Event.GetWatchEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("Win32Select: FD = %d add read event.", Event.GetFD());
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("Win32Select: FD = %d add write event.", Event.GetFD());
    }

    m_EventMap[Event.GetFD()] = std::move(Event);
    QLog::g_Log.WriteInfo("Win32Select: FD = %d add successed, event count = %d.",
        Event.GetFD(),
        static_cast<int>(m_EventMap.size()));

    return true;
}

bool QWin32Select::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
        QLog::g_Log.WriteError("Win32Select: Can not find FD = %d.", Event.GetFD());
        return false;
    }

    if (Event.GetWatchEvents() & QET_READ)
    {
        FD_CLR(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("Win32Select: FD = %d clear read event.", Event.GetFD());
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        FD_CLR(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("Win32Select: FD = %d clear write event.", Event.GetFD());
    }

    m_EventMap.erase(it);
    QLog::g_Log.WriteInfo("Win32Select: FD = %d delete successed, event count = %d.",
        Event.GetFD(),
        static_cast<int>(m_EventMap.size()));

    return true;
}

bool QWin32Select::Dispatch(timeval *tv)
{
    while (!m_IsStop)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        QLog::g_Log.WriteDebug("Win32Select: start...");
        int Result = select(-1, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
        QLog::g_Log.WriteDebug("Win32Select: stop, result = %d.", Result);

        if (Result <= 0)
        {
            QLog::g_Log.WriteError("Win32Select error : %d", WSAGetLastError());
            return false;
        }

        for (int Index = 0; Index < FD_SETSIZE; Index++)
        {
            if (FD_ISSET(m_ReadSetOut.fd_array[Index], &m_ReadSetOut))
            {
                m_EventMap[m_ReadSetOut.fd_array[Index]].CallBack();
            }

            if (FD_ISSET(m_WriteSetOut.fd_array[Index], &m_WriteSetOut))
            {
                m_EventMap[m_WriteSetOut.fd_array[Index]].CallBack();
            }
        }
    }

    return true;
}
