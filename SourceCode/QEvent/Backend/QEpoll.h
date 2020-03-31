#pragma once
#include "QBackend.h"
#include <sys/epoll.h>



class QEpoll : public QBackend
{
public:

    QEpoll();
    virtual ~QEpoll();

    virtual bool AddEvent(int fd, int Event);
    virtual bool DelEvent(int fd, int Event);
    virtual bool Dispatch(struct timeval *tv);

    bool Init(const std::string &BindIP, int Port);

private:

    int                     m_ListenFD;
    int                     m_EpollFD;
    struct epoll_event      m_EventArray[FD_SETSIZE];
};
