#pragma once
#include "QBackend.h"
#include <sys/epoll.h>



class QEpoll : public QBackend
{
public:

    QEpoll();
    virtual ~QEpoll();

    virtual bool AddEvent(const QChannel &Event);
    virtual bool DelEvent(const QChannel &Event);
    virtual bool Dispatch(timeval &tv);

private:

    int                     m_EpollFD;
    struct epoll_event      m_EventArray[FD_SETSIZE];
};
