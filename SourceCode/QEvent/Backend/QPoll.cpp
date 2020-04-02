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
            }

            if (Event.GetWatchEvents() & QET_WRITE)
            {
                m_FDArray[Index].events |= POLLOUT;
            }

            if (m_FDMaxIndex <= Index)
            {
                m_FDMaxIndex = Index + 1;
            }

            m_EventMap[Event.GetFD()] = std::move(Event);
            QLog::g_Log.WriteInfo("Poll : Add new EventFD = %d, max index = %d", Event.GetFD(), m_FDMaxIndex);
            return true;
        }
    }

    QLog::g_Log.WriteError("Poll: Add event failed.");
    return false;
}

bool QPoll::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
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
            QLog::g_Log.WriteInfo("Poll : Delete EventFD = %d", Event.GetFD());
            return true;
        }
    }

    return false;
}

bool QPoll::Dispatch(timeval *tv)
{
    while (!m_IsStop)
    {
        QLog::g_Log.WriteDebug("Start poll...");
        int Result = poll(m_FDArray, m_FDMaxIndex, -1);
        QLog::g_Log.WriteDebug("Stop poll...");

        if (Result <= 0)
        {
            QLog::g_Log.WriteError("Poll error : %s", strerror(errno));
            return false;
        }

        for (int FDIndex = 0; FDIndex < m_FDMaxIndex; FDIndex++)
        {
            if (m_FDArray[FDIndex].revents & POLLIN)
            {
                m_EventMap[m_FDArray[FDIndex].fd].CallBack();
            }

            if(m_FDArray[FDIndex].revents & POLLOUT)
            {
                m_EventMap[m_FDArray[FDIndex].fd].CallBack();
            }
        }
    }

    return true;
}
