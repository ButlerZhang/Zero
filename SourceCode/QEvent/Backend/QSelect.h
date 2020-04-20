#pragma once
#include "QBackend.h"
#include <sys/select.h>



class QSelect : public QBackend
{
public:

    QSelect();
    virtual ~QSelect();

    virtual bool AddEvent(const QChannel &Channel);
    virtual bool DelEvent(const QChannel &Channel);
    virtual bool Dispatch(timeval &tv);

private:

    int                     m_HighestEventFD;
    fd_set                  m_ReadSetIn;
    fd_set                  m_WriteSetIn;
    fd_set                  m_ReadSetOut;
    fd_set                  m_WriteSetOut;
};
