#include "QEpoll.h"
#include "../../QLog/QSimpleLog.h"
#include <unistd.h>                 //close
#include <string.h>                 //strerror



QEpoll::QEpoll()
{
    m_BackendName = "epoll";
    m_EpollFD = epoll_create(FD_SETSIZE);
    QLog::g_Log.WriteInfo("Epoll fd = %d", m_EpollFD);
    memset(m_EventArray, 0, sizeof(m_EventArray));
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
}

bool QEpoll::AddEvent(const QEvent &Event)
{
    epoll_event TempEvent;
    TempEvent.data.fd = Event.GetFD();
    if (Event.GetWatchEvents() & QET_READ)
    {
        TempEvent.events |= EPOLLIN;
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        TempEvent.events |= EPOLLOUT;
    }

    //if (Event.GetWatchEvents() & QET_ET)
    //{
        TempEvent.events |= EPOLLET;
    //}

    if (epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, Event.GetFD(), &TempEvent) != 0)
    {
        QLog::g_Log.WriteError("Epoll: Add new EventFD = %d failed.", Event.GetFD());
        return false;
    }

    m_EventMap[Event.GetFD()] = std::move(Event);
    QLog::g_Log.WriteInfo("Epoll : Add new EventFD = %d", Event.GetFD());
    return true;
}

bool QEpoll::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
        return false;
    }

    epoll_event TempEvent;
    TempEvent.data.fd = Event.GetFD();
    if (epoll_ctl(m_EpollFD, EPOLL_CTL_DEL, Event.GetFD(), &TempEvent) != 0)
    {
        QLog::g_Log.WriteInfo("Epoll : Delete EventFD = %d failed.", Event.GetFD());
        return false;
    }

    m_EventMap.erase(it);
    QLog::g_Log.WriteInfo("Epoll : Delete EventFD = %d", Event.GetFD());
    return true;
}

bool QEpoll::Dispatch(timeval *tv)
{
    while (!m_IsStop)
    {
        QLog::g_Log.WriteDebug("Start Epoll...");
        int ActiveEventCount = epoll_wait(m_EpollFD, m_EventArray, FD_SETSIZE, -1);
        QLog::g_Log.WriteDebug("Stop Epoll...");
        QLog::g_Log.WriteInfo("Epoll acitve event count = %d", ActiveEventCount);

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
