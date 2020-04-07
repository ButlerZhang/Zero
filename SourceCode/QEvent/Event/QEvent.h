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
    virtual ~QEvent();

    inline QEventFD GetFD() const { return m_EventFD; }
    inline void* GetExtendArg() const { return m_ExtendArg; }
    inline int GetWatchEvents() const { return m_WatchEvents; }
    inline int GetResultEvents() const { return m_ResultEvents; }
    inline struct timeval GetTimeOut() const { return m_TimeOut; }
    inline std::shared_ptr<QBackend> GetBackend() const { return m_Backend; }
    inline const CallBackFunction& GetCallBack() const { return m_CallBack; }

    inline void SetTimeOut(const timeval &TimeOut) { m_TimeOut = TimeOut; }
    inline void SetResultEvents(int ResultEvents) { m_ResultEvents = ResultEvents; }
    inline void BindBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

    void CallBack();
    void SetCallBack(CallBackFunction CallBack, void *ExtendArg = NULL);

protected:

    int                                         m_WatchEvents;
    int                                         m_ResultEvents;

    void                                       *m_ExtendArg;
    CallBackFunction                            m_CallBack;

    QEventFD                                    m_EventFD;
    struct timeval                              m_TimeOut;
    std::shared_ptr<QBackend>                   m_Backend;
};
