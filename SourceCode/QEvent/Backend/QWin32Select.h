#pragma once
#include "QBackend.h"



class QWin32Select : public QBackend
{
public:

    QWin32Select();
    virtual ~QWin32Select();

    virtual bool AddEvent(const QChannel &Channel);
    virtual bool DelEvent(const QChannel &Channel);
    virtual bool Dispatch(timeval &tv);

private:

    bool UseSleepSimulateSelect(timeval &tv);

private:

    fd_set                     m_ReadSetIn;
    fd_set                     m_WriteSetIn;
    fd_set                     m_ReadSetOut;
    fd_set                     m_WriteSetOut;
};
