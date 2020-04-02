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
    QET_PERSIST     = 0x10,
    QET_ET          = 0x20
};



class QEvent
{
public:

    QEvent();
    QEvent(QEventFD EventFD, int WatchEvents);
    QEvent(QEventFD EventFD, int WatchEvents, CallBackFunction CallBack);

    virtual ~QEvent();

    inline QEventFD GetFD() const { return m_EventFD; }
    inline int GetWatchEvents() const { return m_WatchEvents; }
    inline int GetResultEvents() const { return m_ResultEvents; }
    inline struct timeval GetTimeOut() const { return m_TimeOut; }
    inline std::shared_ptr<QBackend> GetBackend() const { return m_Backend; }

    void CallBack();
    void SetCallBack(CallBackFunction CallBack) { m_CallBack = std::move(CallBack); }
    void BindBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

protected:

    int                                     m_WatchEvents;
    int                                     m_ResultEvents;
    QEventFD                                m_EventFD;
    struct timeval                          m_TimeOut;
    CallBackFunction                        m_CallBack;
    std::shared_ptr<QBackend>               m_Backend;
};
