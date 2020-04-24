#include "QWin32Select.h"
#include "../QEventLoop.h"
#include "../QLog.h"
#include "../QTimer.h"
#include "../QNetwork.h"



QWin32Select::QWin32Select(QEventLoop &EventLoop) : QBackend(EventLoop)
{
    m_BackendName = "win32select";
    memset(&m_ReadSetIn, 0, sizeof(m_ReadSetIn));
    memset(&m_WriteSetIn, 0, sizeof(m_WriteSetIn));
}

QWin32Select::~QWin32Select()
{
}

bool QWin32Select::AddEvent(const std::shared_ptr<QChannel> &Channel)
{
    if (!QBackend::AddEvent(Channel))
    {
        return false;
    }

    if (Channel->GetEvents() & QET_READ)
    {
        FD_SET(Channel->GetFD(), &m_ReadSetIn);
        g_Log.WriteDebug("win32select: FD = %d add read event, FD count = %d after added.",
            Channel->GetFD(), m_ReadSetIn.fd_count);
    }

    if (Channel->GetEvents() & QET_WRITE)
    {
        FD_SET(Channel->GetFD(), &m_WriteSetIn);
        g_Log.WriteDebug("win32select: FD = %d add write event, FD count = %d after added.",
            Channel->GetFD(), m_WriteSetIn.fd_count);
    }

    return AddEventToChannelMap(Channel, QEO_ADD);
}

bool QWin32Select::DelEvent(const std::shared_ptr<QChannel> &Channel)
{
    if (!QBackend::DelEvent(Channel))
    {
        return false;
    }

    FD_CLR(Channel->GetFD(), &m_ReadSetIn);
    FD_CLR(Channel->GetFD(), &m_WriteSetIn);

    g_Log.WriteDebug("win32select: FD = %d add read event, FD count = %d after deleted.",
        Channel->GetFD(), m_ReadSetIn.fd_count);
    g_Log.WriteDebug("win32select: FD = %d add write event, FD count = %d after deleted.",
        Channel->GetFD(), m_WriteSetIn.fd_count);

    return DelEventFromChannelMap(Channel, QEO_DEL);
}

bool QWin32Select::Dispatch(timeval &tv)
{
    if (UseSleepSimulateSelect(tv))
    {
        return true;
    }

    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    g_Log.WriteDebug("win32select: start...");
    timeval *TempTimeout = QTimer::IsValid(tv) ? &tv : NULL;
    int Result = select(-1, &m_ReadSetOut, &m_WriteSetOut, NULL, TempTimeout);
    g_Log.WriteDebug("win32select: stop, result = %d.", Result);

    if (Result < 0)
    {
        g_Log.WriteError("win32select error : %d", WSAGetLastError());
        m_EventLoop.StopLoop();
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

    long SleepTime = QTimer::ConvertToMillisecond(tv);
    if (SleepTime < 0)
    {
        SleepTime = LONG_MAX;
    }

    Sleep(SleepTime);
    ActiveEvent(m_EventLoop.GetTimer()->GetFD(), 0);
    return true;
}
