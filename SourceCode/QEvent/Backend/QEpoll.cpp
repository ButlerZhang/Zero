#include "QEpoll.h"
#include "../QTime.h"
#include "../../QLog/QSimpleLog.h"

#include <unistd.h>                 //close
#include <string.h>                 //strerror



QEpoll::QEpoll()
{
    m_EpollFD = -1;
    m_BackendName = "epoll";
    memset(m_EventArray, 0, sizeof(m_EventArray));

    m_EpollFD = epoll_create(FD_SETSIZE);
    QLog::g_Log.WriteDebug("epoll: Create epoll fd = %d.", m_EpollFD);
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
}

bool QEpoll::AddEvent(const QChannel &Event)
{
    if (!QBackend::AddEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return AddEventToMapVector(Event, QEO_ADD);
    }

    if (Event.GetEvents() & QET_SIGNAL)
    {
        return m_Signal.Register(Event) && AddEventToMapVector(Event, QEO_ADD);
    }

    if (Event.GetFD() == m_EpollFD)
    {
        return false;
    }

    epoll_event NewEpollEvent;
    memset(&NewEpollEvent, 0, sizeof(epoll_event));

    NewEpollEvent.events |= EPOLLET;
    NewEpollEvent.data.fd = Event.GetFD();

    int EpollOP = EPOLL_CTL_ADD;
    int WatchEvents = Event.GetEvents();

    std::map<QEventFD, std::vector<QChannel>>::iterator MapIt = m_EventMap.find(Event.GetFD());
    if (MapIt != m_EventMap.end())
    {
        for (std::vector<QChannel>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
        {
            EpollOP = EPOLL_CTL_MOD;
            if (VecIt->GetEvents() & QET_READ)
            {
                WatchEvents |= QET_READ;
            }

            if (VecIt->GetEvents() & QET_WRITE)
            {
                WatchEvents |= QET_WRITE;
            }
        }
    }

    if (WatchEvents & QET_READ)
    {
        NewEpollEvent.events |= EPOLLIN;
        QLog::g_Log.WriteDebug("epoll: FD = %d add read event.",
            Event.GetFD());
    }

    if (WatchEvents & QET_WRITE)
    {
        NewEpollEvent.events |= EPOLLOUT;
        QLog::g_Log.WriteDebug("epoll: FD = %d add write event.",
            Event.GetFD());
    }

    if (epoll_ctl(m_EpollFD, EpollOP, Event.GetFD(), &NewEpollEvent) != 0)
    {
        QLog::g_Log.WriteError("epoll: FD = %d op = %d failed, errno = %d, errstr = %s.",
            Event.GetFD(), EpollOP, errno, strerror(errno));
        return false;
    }

    return AddEventToMapVector(Event, static_cast<QEventOption>(EpollOP));
}

bool QEpoll::DelEvent(const QChannel &Event)
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

    int WatchEvents = 0;
    //for (std::vector<QChannel>::iterator VecIt = m_EventMap[Event.GetFD()].begin(); VecIt != m_EventMap[Event.GetFD()].end(); VecIt++)
    //{
    //    if (!VecIt->IsEqual(Event))
    //    {
    //        if (VecIt->GetEvents() & QET_READ)
    //        {
    //            WatchEvents |= QET_READ;
    //        }

    //        if (VecIt->GetEvents() & QET_WRITE)
    //        {
    //            WatchEvents |= QET_WRITE;
    //        }
    //    }
    //}

    epoll_event DelEpollEvent;
    DelEpollEvent.events |= EPOLLET;
    DelEpollEvent.data.fd = Event.GetFD();

    int EpollOP = EPOLL_CTL_DEL;
    if (WatchEvents > 0)
    {
        EpollOP = EPOLL_CTL_MOD;
        DelEpollEvent.events |= WatchEvents;
    }

    if (epoll_ctl(m_EpollFD, EpollOP, Event.GetFD(), &DelEpollEvent) != 0)
    {
        QLog::g_Log.WriteInfo("epoll : FD = %d delete failed, errno = %d, errstr = %s.",
            Event.GetFD(), errno, strerror(errno));
        return false;
    }

    return DelEventFromMapVector(Event, static_cast<QEventOption>(EpollOP));
}

bool QEpoll::Dispatch(timeval &tv)
{
    QLog::g_Log.WriteDebug("epoll: start...");
    int timeout = static_cast<int>(QTime::ConvertToMillisecond(tv));
    int ActiveEventCount = epoll_wait(m_EpollFD, m_EventArray, FD_SETSIZE, timeout);
    QLog::g_Log.WriteDebug("epoll: stop, active event count = %d.", ActiveEventCount);

    if (ActiveEventCount < 0)
    {
        if (errno != EINTR)
        {
            QLog::g_Log.WriteError("epoll error : %s", strerror(errno));
            m_IsStop = true;
            return false;
        }
    }

    if (ActiveEventCount == 0)
    {
        ProcessTimeout();
    }
    else
    {
        for (int Index = 0; Index < ActiveEventCount; Index++)
        {
            int ResultEvents = 0;
            int FD = m_EventArray[Index].data.fd;
            if (m_EventArray[Index].events & EPOLLIN)
            {
                ResultEvents |= QET_READ;
            }

            if (m_EventArray[Index].events & EPOLLOUT)
            {
                ResultEvents |= QET_WRITE;
            }

            if (ResultEvents > 0)
            {
                ActiveEvent(FD, ResultEvents);
            }
        }
    }

    return true;
}
