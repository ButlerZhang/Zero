#include "QPoll.h"
#include "../../QLog/QSimpleLog.h"
#include <string.h>                  //strerror



QPoll::QPoll()
{
    m_FDMaxIndex = 0;
    m_BackendName = "poll";

    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        m_FDArray[Index].fd = -1;
        m_FDArray[Index].events = 0;
        m_FDArray[Index].revents = 0;
    }
}

QPoll::~QPoll()
{
}

bool QPoll::AddEvent(const QEvent &Event)
{
    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        if (m_FDArray[Index].fd < 0)
        {
            m_FDArray[Index].revents = 0;
            m_FDArray[Index].fd = Event.GetFD();

            if (Event.GetWatchEvents() & QET_READ)
            {
                m_FDArray[Index].events |= POLLIN;
                QLog::g_Log.WriteDebug("Poll: FD = %d add read event.", Event.GetFD());
            }

            if (Event.GetWatchEvents() & QET_WRITE)
            {
                m_FDArray[Index].events |= POLLOUT;
                QLog::g_Log.WriteDebug("Poll: FD = %d add write event.", Event.GetFD());
            }

            if (m_FDMaxIndex <= Index)
            {
                m_FDMaxIndex = Index + 1;
            }

            m_EventMap[Event.GetFD()] = std::move(Event);
            QLog::g_Log.WriteInfo("Poll: FD = %d add successed, FDMaxIndex = %d, event count = %d.",
                Event.GetFD(),
                m_FDMaxIndex,
                static_cast<int>(m_EventMap.size()));

            return true;
        }
    }

    QLog::g_Log.WriteError("Poll: FD = %d add failed.", Event.GetFD());
    return false;
}

bool QPoll::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
        QLog::g_Log.WriteError("Poll: Can not find FD = %d.", Event.GetFD());
        return false;
    }

    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        if (m_FDArray[Index].fd == Event.GetFD())
        {
            m_FDArray[Index].fd = -1;
            m_FDArray[Index].events = 0;
            m_FDArray[Index].revents = 0;

            m_EventMap.erase(it);
            QLog::g_Log.WriteInfo("Poll: FD = %d delete successed, FDMaxIndex = %d, event count = %d.",
                Event.GetFD(),
                m_FDMaxIndex,
                static_cast<int>(m_EventMap.size()));

            return true;
        }
    }

    QLog::g_Log.WriteError("Poll: FD = %d delete failed.", Event.GetFD());
    return false;
}

bool QPoll::Dispatch(struct timeval *tv)
{
    QLog::g_Log.WriteDebug("poll: start...");
    int timeout = static_cast<int>(QMinHeap::ConvertToMillisecond(tv));
    int Result = poll(m_FDArray, m_FDMaxIndex, timeout);
    QLog::g_Log.WriteDebug("Select: stop, result = %d.", Result);

    if (Result < 0)
    {
        QLog::g_Log.WriteError("Poll error : %s", strerror(errno));
        m_IsStop = true;
        return false;
    }

    if (Result == 0)
    {
        if (m_EventMap.find(m_TimeFD) != m_EventMap.end())
        {
            m_EventMap[m_TimeFD].CallBack();
        }
    }
    else
    {
        for (int FDIndex = 0; FDIndex < m_FDMaxIndex; FDIndex++)
        {
            if (m_FDArray[FDIndex].revents & POLLIN)
            {
                m_EventMap[m_FDArray[FDIndex].fd].CallBack();
            }

            if (m_FDArray[FDIndex].revents & POLLOUT)
            {
                m_EventMap[m_FDArray[FDIndex].fd].CallBack();
            }
        }
    }

    return true;
}
