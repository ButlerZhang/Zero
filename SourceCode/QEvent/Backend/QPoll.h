#pragma once
#include "QBackend.h"
#include <sys/poll.h>



class QPoll : public QBackend
{
public:

    QPoll(QEventLoop &EventLoop);
    virtual ~QPoll();

    virtual bool AddEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool DelEvent(const std::shared_ptr<QChannel> &Channel);
    virtual bool Dispatch(timeval &tv);

private:

    int                     m_FDMaxIndex;
    struct pollfd           m_FDArray[FD_SETSIZE];
};
