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
    if (!QBackend::AddEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        m_EventMap[m_TimerFD].push_back(std::move(Event));
        WriteEventOperationLog(m_TimerFD, Event.GetFD(), QEO_ADD);
        m_MinHeap.AddTimeOut(Event, m_TimerFD, m_EventMap[m_TimerFD].size() - 1);
        return true;
    }

    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        if (m_FDArray[Index].fd < 0 || m_FDArray[Index].fd == Event.GetFD())
        {
            QEventOption OP = (m_FDArray[Index].fd < 0) ? QEO_ADD : QEO_MOD;

            m_FDArray[Index].revents = 0;
            m_FDArray[Index].fd = Event.GetFD();

            if (Event.GetEvents() & QET_READ)
            {
                m_FDArray[Index].events |= POLLIN;
                QLog::g_Log.WriteDebug("poll: FD = %d add read event.", Event.GetFD());
            }

            if (Event.GetEvents() & QET_WRITE)
            {
                m_FDArray[Index].events |= POLLOUT;
                QLog::g_Log.WriteDebug("poll: FD = %d add write event.", Event.GetFD());
            }

            if (m_FDMaxIndex <= Index)
            {
                m_FDMaxIndex = Index + 1;
                QLog::g_Log.WriteDebug("poll: FD max index = %d.", m_FDMaxIndex);
            }

            m_EventMap[Event.GetFD()].push_back(std::move(Event));
            WriteEventOperationLog(Event.GetFD(), Event.GetFD(), OP);
            m_MinHeap.AddTimeOut(Event, Event.GetFD(), m_EventMap[Event.GetFD()].size() - 1);
            return true;
        }
    }

    QLog::g_Log.WriteError("%s: FD = %d watch events = %d add failed, no location.",
        m_BackendName.c_str(), Event.GetFD(), Event.GetEvents());
    return false;
}

bool QPoll::DelEvent(const QEvent &Event)
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

    std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(Event.GetFD());
    if (MapIt == m_EventMap.end())
    {
        for (int Index = 0; Index < FD_SETSIZE; Index++)
        {
            if (m_FDArray[Index].fd == Event.GetFD())
            {
                int LastIndex = m_FDMaxIndex - 1;
                if (LastIndex == Index)
                {
                    m_FDArray[Index].fd = -1;
                    m_FDArray[Index].events = 0;
                    m_FDArray[Index].revents = 0;

                    m_FDMaxIndex = Index;
                }
                else
                {
                    m_FDArray[Index].fd = m_FDArray[LastIndex].fd;
                    m_FDArray[Index].events = m_FDArray[LastIndex].events;
                    m_FDArray[Index].revents = m_FDArray[LastIndex].revents;

                    m_FDArray[LastIndex].fd = -1;
                    m_FDArray[LastIndex].events = 0;
                    m_FDArray[LastIndex].revents = 0;

                    m_FDMaxIndex = LastIndex;
                }

                QLog::g_Log.WriteDebug("poll: FD max index = %d after deleted.", m_FDMaxIndex);
                WriteEventOperationLog(Event.GetFD(), Event.GetFD(), QEO_DEL);
                return true;
            }
        }
    }
    else
    {
        for (int Index = 0; Index < FD_SETSIZE; Index++)
        {
            if (m_FDArray[Index].fd == Event.GetFD())
            {
                m_FDArray[Index].events = 0;

                for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
                {
                    if (VecIt->GetEvents() & QET_READ)
                    {
                        m_FDArray[Index].events |= POLLIN;
                    }

                    if (VecIt->GetEvents() & QET_WRITE)
                    {
                        m_FDArray[Index].events |= POLLOUT;
                    }
                }

                QLog::g_Log.WriteDebug("poll: FD max index = %d after modify.", m_FDMaxIndex);
                WriteEventOperationLog(Event.GetFD(), Event.GetFD(), QEO_MOD);
                return true;
            }
        }
    }

    QLog::g_Log.WriteError("%s: FD = %d watch events = %d deleted failed, no location.",
        m_BackendName.c_str(), Event.GetFD(), Event.GetEvents());
    return false;
}

bool QPoll::Dispatch(struct timeval *tv)
{
    QLog::g_Log.WriteDebug("poll: start...");
    int timeout = static_cast<int>(QTime::ConvertToMillisecond(tv));
    int Result = poll(m_FDArray, m_FDMaxIndex, timeout);
    QLog::g_Log.WriteDebug("poll: stop, result = %d.", Result);

    if (Result < 0)
    {
        QLog::g_Log.WriteError("poll error : %s", strerror(errno));
        m_IsStop = true;
        return false;
    }

    if (Result == 0)
    {
        ProcessTimeOut(tv);
    }
    else
    {
        for (int FDIndex = 0; FDIndex < m_FDMaxIndex; FDIndex++)
        {
            int ResultEvents = 0;
            if (m_FDArray[FDIndex].revents & POLLIN)
            {
                ResultEvents |= QET_READ;
            }

            if (m_FDArray[FDIndex].revents & POLLOUT)
            {
                ResultEvents |= QET_WRITE;
            }

            ActiveEvent(m_FDArray[FDIndex].fd, ResultEvents);
        }
    }

    return true;
}
