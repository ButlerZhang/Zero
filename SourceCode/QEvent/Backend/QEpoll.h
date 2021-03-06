#pragma once
#include "QBackend.h"
#include <sys/epoll.h>



class QEpoll : public QBackend
{
public:

    QEpoll(QEventLoop &EventLoop);
    virtual ~QEpoll();

    virtual bool AddEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool DelEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool Dispatch(timeval &tv);

private:

    int                     m_EpollFD;
    struct epoll_event      m_EventArray[FD_SETSIZE];
};
