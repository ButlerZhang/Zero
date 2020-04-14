#include "QPoll.h"
#include "../Tools/QTime.h"
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

    if (AddTimeoutEvent(Event))
    {
        return true;
    }

    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        if (m_FDArray[Index].fd < 0 || m_FDArray[Index].fd == Event.GetFD())
        {
            QEventOption OP = (m_FDArray[Index].fd < 0) ? QEO_ADD : QEO_MOD;
            m_FDArray[Index].fd = Event.GetFD();
            m_FDArray[Index].revents = 0;

            if (Event.GetEvents() & QET_READ)
            {
                m_FDArray[Index].events |= POLLIN;
                QLog::g_Log.WriteDebug("poll: FD = %d add read event.",
                    Event.GetFD());
            }

            if (Event.GetEvents() & QET_WRITE)
            {
                m_FDArray[Index].events |= POLLOUT;
                QLog::g_Log.WriteDebug("poll: FD = %d add write event.",
                    Event.GetFD());
            }

            if (m_FDMaxIndex <= Index)
            {
                m_FDMaxIndex = Index + 1;
                QLog::g_Log.WriteDebug("poll: FD max index = %d after added.",
                    m_FDMaxIndex);
            }

            return AddEventToMapVector(Event, OP);
        }
    }

    QLog::g_Log.WriteError("poll: FD = %d, events = %d add failed, no location.",
        Event.GetFD(), Event.GetEvents());
    return false;
}

bool QPoll::DelEvent(const QEvent &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    if (DelTimeoutEvent(Event))
    {
        return true;
    }

    int DeleteIndex = -1;
    for (int Index = 0; Index < FD_SETSIZE; Index++)
    {
        if (m_FDArray[Index].fd == Event.GetFD())
        {
            DeleteIndex = Index;
            m_FDArray[Index].fd = -1;
            m_FDArray[Index].events = 0;
            m_FDArray[Index].revents = 0;

            for (std::vector<QEvent>::iterator VecIt = m_EventMap[Event.GetFD()].begin(); VecIt != m_EventMap[Event.GetFD()].end(); VecIt++)
            {
                if (!VecIt->IsEqual(Event))
                {
                    m_FDArray[Index].fd = Event.GetFD();
                    if (VecIt->GetEvents() & QET_READ)
                    {
                        m_FDArray[Index].events |= POLLIN;
                    }

                    if (VecIt->GetEvents() & QET_WRITE)
                    {
                        m_FDArray[Index].events |= POLLOUT;
                    }
                }
            }

            break;
        }
    }

    if (DeleteIndex >= 0 && m_FDArray[DeleteIndex].fd < 0)
    {
        m_FDMaxIndex -= 1;
        if (DeleteIndex < m_FDMaxIndex)
        {
            m_FDArray[DeleteIndex].fd = m_FDArray[m_FDMaxIndex].fd;
            m_FDArray[DeleteIndex].events = m_FDArray[m_FDMaxIndex].events;
            m_FDArray[DeleteIndex].revents = m_FDArray[m_FDMaxIndex].revents;

            m_FDArray[m_FDMaxIndex].fd = -1;
            m_FDArray[m_FDMaxIndex].events = 0;
            m_FDArray[m_FDMaxIndex].revents = 0;
        }
    }

    QLog::g_Log.WriteDebug("poll: FD max index = %d after deleted.",
        m_FDMaxIndex);

    return DelEventFromMapVector(Event);
}

bool QPoll::Dispatch(timeval &tv)
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
        ProcessTimeout();
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
