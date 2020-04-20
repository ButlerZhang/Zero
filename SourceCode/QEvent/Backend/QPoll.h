#pragma once
#include "QBackend.h"
#include <sys/poll.h>



class QPoll : public QBackend
{
public:

    QPoll();
    virtual ~QPoll();

    virtual bool AddEvent(const QChannel &Event);
    virtual bool DelEvent(const QChannel &Event);
    virtual bool Dispatch(timeval &tv);

private:

    int                     m_FDMaxIndex;
    struct pollfd           m_FDArray[FD_SETSIZE];
};
