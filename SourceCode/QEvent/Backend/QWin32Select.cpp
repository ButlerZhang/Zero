#include "QWin32Select.h"
#include "../../QLog/QSimpleLog.h"
#include "../QNetwork.h"
#include "../QTime.h"



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
        return AddEventToMapVector(Event, QEO_ADD);
    }

    if (Event.GetEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("win32select: FD = %d add read event, FD count = %d after added.",
            Event.GetFD(), m_ReadSetIn.fd_count);
    }

    if (Event.GetEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("win32select: FD = %d add write event, FD count = %d after added.",
            Event.GetFD(), m_WriteSetIn.fd_count);
    }

    return AddEventToMapVector(Event, QEO_ADD);
}

bool QWin32Select::DelEvent(const QEvent &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return DelEventFromMapVector(Event, QEO_DEL);
    }

    FD_CLR(Event.GetFD(), &m_ReadSetIn);
    FD_CLR(Event.GetFD(), &m_WriteSetIn);

    for (std::vector<QEvent>::iterator VecIt = m_EventMap[Event.GetFD()].begin(); VecIt != m_EventMap[Event.GetFD()].end(); VecIt++)
    {
        if (!VecIt->IsEqual(Event))
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

    QLog::g_Log.WriteDebug("win32select: FD = %d add read event, FD count = %d after deleted.",
        Event.GetFD(), m_ReadSetIn.fd_count);
    QLog::g_Log.WriteDebug("win32select: FD = %d add write event, FD count = %d after deleted.",
        Event.GetFD(), m_WriteSetIn.fd_count);

    return DelEventFromMapVector(Event, QEO_DEL);
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
