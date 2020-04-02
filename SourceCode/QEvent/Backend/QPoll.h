#pragma once
#include "QBackend.h"
#include <sys/poll.h>



class QPoll : public QBackend
{
public:

    QPoll();
    virtual ~QPoll();

    virtual bool AddEvent(const QEvent &Event);
    virtual bool DelEvent(const QEvent &Event);
    virtual bool Dispatch(struct timeval *tv);

private:

    int                     m_FDMaxIndex;
    struct pollfd           m_FDArray[FD_SETSIZE];
};
