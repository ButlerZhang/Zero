#pragma once
#include "QBackend.h"



class QWin32Select : public QBackend
{
public:

    QWin32Select();
    virtual ~QWin32Select();

    virtual bool AddEvent(const QEvent &Event);
    virtual bool DelEvent(const QEvent &Event);
    virtual bool Dispatch(struct timeval *tv);

private:

    bool UseSleepSimulateSelect(struct timeval *tv);

private:

    fd_set                     m_ReadSetIn;
    fd_set                     m_WriteSetIn;
    fd_set                     m_ReadSetOut;
    fd_set                     m_WriteSetOut;
};
