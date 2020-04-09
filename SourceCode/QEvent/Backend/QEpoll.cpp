#include "QEpoll.h"
#include "../../QLog/QSimpleLog.h"
#include <unistd.h>                 //close
#include <string.h>                 //strerror



QEpoll::QEpoll()
{
    m_EpollFD = -1;
    m_BackendName = "epoll";
    memset(m_EventArray, 0, sizeof(m_EventArray));

    m_EpollFD = epoll_create(FD_SETSIZE);
    QLog::g_Log.WriteInfo("epoll: Create epoll fd = %d.", m_EpollFD);
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
}

bool QEpoll::AddEvent(const QEvent &Event)
{
    if (!QBackend::AddEvent(Event))
    {
        return false;
    }

    int EpollOP = EPOLL_CTL_ADD;
    if (Event.GetFD() >= 0)
    {
        epoll_event NewEpollEvent;
        memset(&NewEpollEvent, 0, sizeof(epoll_event));

        NewEpollEvent.events |= EPOLLET;
        NewEpollEvent.data.fd = Event.GetFD();

        int WatchEvents = Event.GetEvents();
        std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(Event.GetFD());
        if (MapIt != m_EventMap.end())
        {
            for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
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
            QLog::g_Log.WriteDebug("epoll: FD = %d add read event.", Event.GetFD());
        }

        if (WatchEvents & QET_WRITE)
        {
            NewEpollEvent.events |= EPOLLOUT;
            QLog::g_Log.WriteDebug("epoll: FD = %d add write event.", Event.GetFD());
        }

        if (epoll_ctl(m_EpollFD, EpollOP, Event.GetFD(), &NewEpollEvent) != 0)
        {
            QLog::g_Log.WriteError("epoll: FD = %d op = %d failed, errno = %d, errstr = %s.",
                Event.GetFD(), EpollOP, errno, strerror(errno));
            return false;
        }
    }

    m_EventMap[Event.GetFD()].push_back(std::move(Event));
    WriteEventOperationLog(Event.GetFD(), static_cast<QEventOption>(EpollOP));
    return true;
}

bool QEpoll::DelEvent(const QEvent &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    epoll_event DelEpollEvent;
    DelEpollEvent.data.fd = Event.GetFD();

    int WatchEvents = 0;
    int EpollOP = EPOLL_CTL_DEL;

    std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(Event.GetFD());
    if (MapIt != m_EventMap.end())
    {
        for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
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

    if (epoll_ctl(m_EpollFD, EpollOP, Event.GetFD(), &DelEpollEvent) != 0)
    {
        QLog::g_Log.WriteInfo("epoll : FD = %d delete failed, errno = %d, errstr = %s.",
            Event.GetFD(), errno, strerror(errno));
        return false;
    }

    WriteEventOperationLog(Event.GetFD(), static_cast<QEventOption>(EpollOP));
    return true;
}

bool QEpoll::Dispatch(struct timeval *tv)
{
    QLog::g_Log.WriteDebug("epoll: start...");
    int timeout = static_cast<int>(QMinHeap::ConvertToMillisecond(tv));
    int ActiveEventCount = epoll_wait(m_EpollFD, m_EventArray, FD_SETSIZE, timeout);
    QLog::g_Log.WriteDebug("epoll: stop, active event count = %d.", ActiveEventCount);

    if (ActiveEventCount < 0)
    {
        QLog::g_Log.WriteError("epoll error : %s", strerror(errno));
        m_IsStop = true;
        return false;
    }

    if (ActiveEventCount == 0)
    {
        if (m_EventMap.find(m_TimerFD) != m_EventMap.end())
        {
            m_EventMap[m_TimerFD][0].CallBack();
        }
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

            ActiveEvent(FD, ResultEvents);
        }
    }

    return true;
}
