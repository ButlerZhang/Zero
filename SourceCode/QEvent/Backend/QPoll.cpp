#include "QPoll.h"
#include "../QTime.h"
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

bool QPoll::AddEvent(const QChannel &Event)
{
    if (!QBackend::AddEvent(Event))
    {
        return false;
    }

    //if (Event.GetEvents() & QET_TIMEOUT)
    //{
    //    return AddEventToMapVector(Event, QEO_ADD);
    //}

    //if (Event.GetEvents() & QET_SIGNAL)
    //{
    //    return m_Signal.Register(Event) && AddEventToMapVector(Event, QEO_ADD);
    //}

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

bool QPoll::DelEvent(const QChannel &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return DelEventFromMapVector(Event, QEO_DEL);
    }

    if (Event.GetEvents() & QET_SIGNAL)
    {
        return m_Signal.CancelRegister(Event) && DelEventFromMapVector(Event, QEO_DEL);
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

            //for (std::vector<QChannel>::iterator VecIt = m_ChannelMap[Event.GetFD()].begin(); VecIt != m_ChannelMap[Event.GetFD()].end(); VecIt++)
            //{
            //    if (!VecIt->IsEqual(Event))
            //    {
            //        m_FDArray[Index].fd = Event.GetFD();
            //        if (VecIt->GetEvents() & QET_READ)
            //        {
            //            m_FDArray[Index].events |= POLLIN;
            //        }

            //        if (VecIt->GetEvents() & QET_WRITE)
            //        {
            //            m_FDArray[Index].events |= POLLOUT;
            //        }
            //    }
            //}

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

    return DelEventFromMapVector(Event, QEO_DEL);
}

bool QPoll::Dispatch(timeval &tv)
{
    QLog::g_Log.WriteDebug("poll: start...");
    int timeout = static_cast<int>(QTime::ConvertToMillisecond(tv));
    int Result = poll(m_FDArray, m_FDMaxIndex, timeout);
    QLog::g_Log.WriteDebug("poll: stop, result = %d.", Result);

    if (Result < 0)
    {
        if (errno != EINTR)
        {
            QLog::g_Log.WriteError("poll error : %s", strerror(errno));
            m_IsStop = true;
            return false;
        }
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

            if (ResultEvents > 0)
            {
                ActiveEvent(m_FDArray[FDIndex].fd, ResultEvents);
            }
        }
    }

    return true;
}
