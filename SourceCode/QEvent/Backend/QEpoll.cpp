#include "QEpoll.h"
#include "../../QLog/QSimpleLog.h"

#include <unistd.h>                 //close
#include <string.h>                 //strerror



QEpoll::QEpoll()
{
    m_BackendName = "epoll";
    m_EpollFD = epoll_create(FD_SETSIZE);
    memset(m_EventArray, 0, sizeof(m_EventArray));
}

QEpoll::~QEpoll()
{
    close(m_EpollFD);
}

bool QEpoll::AddEvent(const QChannel &Channel)
{
    if (!QBackend::AddEvent(Channel))
    {
        return false;
    }

    if (Channel.GetFD() == m_EpollFD)
    {
        return false;
    }

    epoll_event NewEpollEvent;
    memset(&NewEpollEvent, 0, sizeof(epoll_event));

    NewEpollEvent.events |= EPOLLET;
    NewEpollEvent.data.fd = Channel.GetFD();

    if (Channel.GetEvents() & QET_READ)
    {
        NewEpollEvent.events |= EPOLLIN;
        QLog::g_Log.WriteDebug("epoll: FD = %d add read event.",
            Channel.GetFD());
    }

    if (Channel.GetEvents() & QET_WRITE)
    {
        NewEpollEvent.events |= EPOLLOUT;
        QLog::g_Log.WriteDebug("epoll: FD = %d add write event.",
            Channel.GetFD());
    }

    if (epoll_ctl(m_EpollFD, EPOLL_CTL_ADD, Channel.GetFD(), &NewEpollEvent) != 0)
    {
        QLog::g_Log.WriteError("epoll: FD = %d op = %d failed, errno = %d, errstr = %s.",
            Channel.GetFD(), EPOLL_CTL_ADD, errno, strerror(errno));
        return false;
    }

    return AddEventToChannelMap(Channel, static_cast<QEventOption>(EPOLL_CTL_ADD));
}

bool QEpoll::DelEvent(const QChannel &Channel)
{
    if (!QBackend::DelEvent(Channel))
    {
        return false;
    }

    if (Channel.GetFD() == m_EpollFD)
    {
        return false;
    }

    epoll_event DelEpollEvent;
    DelEpollEvent.events |= EPOLLET;
    DelEpollEvent.data.fd = Channel.GetFD();

    if (epoll_ctl(m_EpollFD, EPOLL_CTL_DEL, Channel.GetFD(), &DelEpollEvent) != 0)
    {
        QLog::g_Log.WriteInfo("epoll : FD = %d delete failed, errno = %d, errstr = %s.",
            Channel.GetFD(), errno, strerror(errno));
        return false;
    }

    return DelEventFromChannelMap(Channel, static_cast<QEventOption>(EPOLL_CTL_DEL));
}

bool QEpoll::Dispatch(timeval &tv)
{
    QLog::g_Log.WriteDebug("epoll: start...");
    int timeout = static_cast<int>(QTimer::ConvertToMillisecond(tv));
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
        ActiveEvent(m_Timer.GetFD(), 0);
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
