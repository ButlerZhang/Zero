#pragma once
#include "QBackend.h"
#include <sys/poll.h>



class QPoll : public QBackend
{
public:

    QPoll();
    virtual ~QPoll();

    virtual bool AddEvent(int fd, int Event);
    virtual bool DelEvent(int fd, int Event);
    virtual bool Dispatch(struct timeval *tv);

    bool Init(const std::string &BindIP, int Port);

private:

    int                     m_ListenFD;
    struct pollfd           m_FDArray[FD_SETSIZE];
};