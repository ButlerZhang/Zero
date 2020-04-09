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
    if (!QBackend::AddEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        m_EventMap[m_TimerFD].push_back(std::move(Event));
        WriteEventOperationLog(m_TimerFD, Event.GetFD(), QEO_ADD);
        return true;
    }

    if (Event.GetEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("win32select: FD = %d add read event.", Event.GetFD());
    }

    if (Event.GetEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("win32select: FD = %d add write event.", Event.GetFD());
    }

    m_EventMap[Event.GetFD()].push_back(std::move(Event));
    WriteEventOperationLog(Event.GetFD(), Event.GetFD(), QEO_ADD);
    return true;
}

bool QWin32Select::DelEvent(const QEvent &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        WriteEventOperationLog(m_TimerFD, Event.GetFD(), QEO_DEL);
        return true;
    }

    QEventOption OP = QEO_DEL;
    FD_CLR(Event.GetFD(), &m_ReadSetIn);
    FD_CLR(Event.GetFD(), &m_WriteSetIn);

    std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(Event.GetFD());
    if (MapIt != m_EventMap.end())
    {
        OP = QEO_MOD;
        for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
        {
            if (VecIt->GetEvents() & QET_READ)
            {
                FD_SET(VecIt->GetFD(), &m_ReadSetIn);
            }

            if (VecIt->GetEvents() & QET_WRITE)
            {
                FD_SET(VecIt->GetFD(), &m_WriteSetIn);
            }
        }
    }

    WriteEventOperationLog(Event.GetFD(), Event.GetFD(), OP);
    return true;
}

bool QWin32Select::Dispatch(struct timeval *tv)
{
    if (UseSleepSimulateSelect(tv))
    {
        return true;
    }

    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    QLog::g_Log.WriteDebug("win32select: start...");
    int Result = select(-1, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
    QLog::g_Log.WriteDebug("win32select: stop, result = %d.", Result);

    if (Result < 0)
    {
        QLog::g_Log.WriteError("win32select error : %d", WSAGetLastError());
        m_IsStop = true;
        return false;
    }

    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        if (FD_ISSET(m_ReadSetOut.fd_array[Index], &m_ReadSetOut))
        {
            ActiveEvent(m_ReadSetOut.fd_array[Index], QET_READ);
        }

        if (FD_ISSET(m_WriteSetOut.fd_array[Index], &m_WriteSetOut))
        {
            ActiveEvent(m_WriteSetOut.fd_array[Index], QET_WRITE);
        }
    }

    return true;
}

bool QWin32Select::UseSleepSimulateSelect(struct timeval *tv)
{
    int fdCount = m_ReadSetIn.fd_count > m_WriteSetIn.fd_count ? m_ReadSetIn.fd_count : m_WriteSetIn.fd_count;
    if (fdCount > 0)
    {
        return false;
    }

    long SleepTime = QMinHeap::ConvertToMillisecond(tv);
    if (SleepTime < 0)
    {
        SleepTime = LONG_MAX;
    }

    Sleep(SleepTime);

    if (m_EventMap.find(m_TimerFD) != m_EventMap.end())
    {
        m_EventMap[m_TimerFD][0].CallBack();
    }

    return true;
}
