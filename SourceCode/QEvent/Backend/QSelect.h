#pragma once
#include "QBackend.h"



class QSelect : public QBackend
{
public:

    QSelect();
    virtual ~QSelect();

    virtual bool AddEvent(int fd, int Event);
    virtual bool DelEvent(int fd, int Event);
    virtual bool Dispatch(struct timeval *tv);

    bool Init(const std::string &BindIP, int Port);

private:

    int                     m_ListenFD;
    int                     m_HighestEventFD;
    fd_set                  m_ReadSetIn;
    fd_set                  m_WriteSetIn;
    fd_set                  m_ReadSetOut;
    fd_set                  m_WriteSetOut;
};
