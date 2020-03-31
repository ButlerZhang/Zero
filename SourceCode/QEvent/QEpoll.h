#pragma once
#include <string>



class QEpoll
{
public:

    QEpoll();
    ~QEpoll();

    bool Init(const std::string &BindIP, int Port);
    bool Dispatch(struct timeval* tv);

private:

    int                     m_ListenFD;
    int                     m_EpollFD;
};
