#pragma once
#include "../QLibBase.h"
#include <memory>
#include <functional>

class QEvent;
class QBackend;

typedef std::function<void(const QEvent &Event)> CallBackFunction;

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

    QEvent() {}
    QEvent(QSOCKET EventFD, int Events);
    virtual ~QEvent();

    inline int GetEvents() const { return m_Events; }
    inline int GetResultEvents() const { return m_ResultEvents; }

    inline QSOCKET GetFD() const { return m_EventFD; }
    inline struct timeval GetTimeOut() const { return m_TimeOut; }

    void CallBack();
    void SetCallBack(CallBackFunction CallBack) { m_CallBack = std::move(CallBack); }
    void BindBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

protected:

    int                                     m_Events;
    int                                     m_ResultEvents;
    QSOCKET                                 m_EventFD;
    struct timeval                          m_TimeOut;
    CallBackFunction                        m_CallBack;
    std::shared_ptr<QBackend>               m_Backend;
};
