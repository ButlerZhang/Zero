#pragma once
#include <string>
#include <sys/epoll.h>



class QEpoll
{
public:

    QEpoll();
    ~QEpoll();

    bool Init(const std::string &BindIP, int Port);
    bool Dispatch(struct timeval* tv);

    const std::string& GetEngineName() const { return m_EngineName; }

private:

    int                     m_ListenFD;
    int                     m_EpollFD;
    std::string             m_EngineName;
    struct epoll_event      m_EventArray[FD_SETSIZE];
};
