#pragma once
#include "../QLibBase.h"
#include <memory>

class QBackend;

enum QEventType
{
    QET_TIMEOUT     = 0x01,
    QET_READ        = 0x02,
    QET_WRITE       = 0x04,
    QET_SIGNAL      = 0x08,
    QET_PERSIST     = 0x10
};



class QEvent
{
public:

    QEvent(QSOCKET EventFD, int Events);
    virtual ~QEvent();
    virtual void CallBack();

    int GetEvents() const { return m_Events; }
    int GetResultEvents() const { return m_ResultEvents; }

    QSOCKET GetFD() const { return m_EventFD; }
    struct timeval GetTimeOut() const { return m_TimeOut; }

    void BindBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

protected:

    int                                     m_Events;
    int                                     m_ResultEvents;
    QSOCKET                                 m_EventFD;
    struct timeval                          m_TimeOut;
    std::shared_ptr<QBackend>               m_Backend;
};
