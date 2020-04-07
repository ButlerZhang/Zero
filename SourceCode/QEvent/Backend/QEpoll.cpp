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
    QLog::g_Log.WriteInfo("Epoll: Create epoll fd = %d.", m_EpollFD);
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
}

bool QEpoll::AddEvent(const QEvent &Event)
{
    if (Event.GetFD() >= 0)
    {
        epoll_event NewEpollEvent;
        memset(&NewEpollEvent, 0, sizeof(epoll_event));

        NewEpollEvent.events |= EPOLLET;
        NewEpollEvent.data.fd = Event.GetFD();

        if (Event.GetWatchEvents() & QET_READ)
        {
            NewEpollEvent.events |= EPOLLIN;
            QLog::g_Log.WriteDebug("Epoll: FD = %d add read event.", Event.GetFD());
        }

        if (Event.GetWatchEvents() & QET_WRITE)
        {
            NewEpollEvent.events |= EPOLLOUT;
            QLog::g_Log.WriteDebug("Epoll: FD = %d add write event.", Event.GetFD());
        }

        if (epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, Event.GetFD(), &NewEpollEvent) != 0)
        {
            QLog::g_Log.WriteError("Epoll: FD = %d add failed, errno = %d, errstr = %s.",
                Event.GetFD(), errno, strerror(errno));
            return false;
        }
    }

    m_EventMap[Event.GetFD()] = std::move(Event);
    QLog::g_Log.WriteInfo("Epoll: FD = %d add successed, event count = %d.",
        Event.GetFD(),
        static_cast<int>(m_EventMap.size()));

    return true;
}

bool QEpoll::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
        QLog::g_Log.WriteError("Epoll: Can not find FD = %d.", Event.GetFD());
        return false;
    }

    epoll_event DelEpollEvent;
    DelEpollEvent.data.fd = Event.GetFD();
    if (epoll_ctl(m_EpollFD, EPOLL_CTL_DEL, Event.GetFD(), &DelEpollEvent) != 0)
    {
        QLog::g_Log.WriteInfo("Epoll : FD = %d delete failed.", Event.GetFD());
        return false;
    }

    m_EventMap.erase(it);
    QLog::g_Log.WriteInfo("Epoll: FD = %d delete successed, event count = %d.",
        Event.GetFD(),
        static_cast<int>(m_EventMap.size()));

    return true;
}

bool QEpoll::Dispatch(struct timeval *tv)
{
    QLog::g_Log.WriteDebug("Epoll: start...");
    int timeout = static_cast<int>(QMinHeap::ConvertToMillisecond(tv));
    int ActiveEventCount = epoll_wait(m_EpollFD, m_EventArray, FD_SETSIZE, timeout);
    QLog::g_Log.WriteDebug("Epoll: stop, active event count = %d.", ActiveEventCount);

    if (ActiveEventCount < 0)
    {
        QLog::g_Log.WriteError("Epoll error : %s", strerror(errno));
        m_IsStop = true;
        return false;
    }

    if (ActiveEventCount == 0)
    {
        if (m_EventMap.find(m_TimeFD) != m_EventMap.end())
        {
            m_EventMap[m_TimeFD].CallBack();
        }
    }
    else
    {
        for (int Index = 0; Index < ActiveEventCount; Index++)
        {
            int FD = m_EventArray[Index].data.fd;
            if (m_EventArray[Index].events & EPOLLIN)
            {
                m_EventMap[FD].CallBack();
            }

            if (m_EventArray[Index].events & EPOLLOUT)
            {
                m_EventMap[FD].CallBack();
            }
        }
    }

    return true;
}
