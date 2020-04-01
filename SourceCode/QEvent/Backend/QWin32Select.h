#pragma once
#include "QBackend.h"
#include <WinSock2.h>



class QWin32Select : public QBackend
{
public:

    QWin32Select();
    virtual ~QWin32Select();

    virtual bool AddEvent(const QEvent &Event);
    virtual bool DelEvent(const QEvent &Event);
    virtual bool Dispatch(struct timeval *tv);

    bool Init(const std::string &BindIP, int Port);

private:

    QSOCKET                    m_ListenFD;
    fd_set                     m_ReadSetIn;
    fd_set                     m_WriteSetIn;
    fd_set                     m_ReadSetOut;
    fd_set                     m_WriteSetOut;
};
