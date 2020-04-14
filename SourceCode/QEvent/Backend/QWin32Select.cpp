#include "QWin32Select.h"
#include "../../QLog/QSimpleLog.h"
#include "../Network/QNetwork.h"
#include "../Tools/QTime.h"



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

    if (AddTimeoutEvent(Event))
    {
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

    return AddEventToMapVector(Event, QEO_ADD);
}

bool QWin32Select::DelEvent(const QEvent &Event)
{
    QEvent CopyEvent = Event;
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    if (CopyEvent.GetEvents() & QET_TIMEOUT)
    {
        WriteEventOperationLog(m_TimerFD, CopyEvent.GetFD(), QEO_DEL);
        return true;
    }

    QEventOption OP = QEO_DEL;
    FD_CLR(CopyEvent.GetFD(), &m_ReadSetIn);
    FD_CLR(CopyEvent.GetFD(), &m_WriteSetIn);

    std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(CopyEvent.GetFD());
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

    WriteEventOperationLog(CopyEvent.GetFD(), CopyEvent.GetFD(), OP);
    return true;
}

bool QWin32Select::Dispatch(timeval &tv)
{
    if (UseSleepSimulateSelect(tv))
    {
        return true;
    }

    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    QLog::g_Log.WriteDebug("win32select: start...");
    timeval *TempTimeout = QTime::IsValid(tv) ? &tv : NULL;
    int Result = select(-1, &m_ReadSetOut, &m_WriteSetOut, NULL, TempTimeout);
    QLog::g_Log.WriteDebug("win32select: stop, result = %d.", Result);

    if (Result < 0)
    {
        QLog::g_Log.WriteError("win32select error : %d", WSAGetLastError());
        m_IsStop = true;
        return false;
    }

    for (u_int Index = 0; Index < m_ReadSetOut.fd_count; Index++)
    {
        if (FD_ISSET(m_ReadSetOut.fd_array[Index], &m_ReadSetOut))
        {
            ActiveEvent(m_ReadSetOut.fd_array[Index], QET_READ);
        }
    }

    for (u_int Index = 0; Index < m_WriteSetOut.fd_count; Index++)
    {
        if (FD_ISSET(m_WriteSetOut.fd_array[Index], &m_WriteSetOut))
        {
            ActiveEvent(m_WriteSetOut.fd_array[Index], QET_WRITE);
        }
    }

    return true;
}

bool QWin32Select::UseSleepSimulateSelect(timeval &tv)
{
    if (m_ReadSetIn.fd_count > 0 || m_WriteSetIn.fd_count > 0)
    {
        return false;
    }

    if (!m_MinHeap.HasNode())
    {
        return false;
    }

    long SleepTime = QTime::ConvertToMillisecond(tv);
    if (SleepTime < 0)
    {
        SleepTime = LONG_MAX;
    }

    Sleep(SleepTime);
    ProcessTimeout();
    return true;
}
