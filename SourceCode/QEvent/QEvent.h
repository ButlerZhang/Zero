#pragma once
#include "QLibBase.h"
#include "QEventEnum.h"

#include <memory>
#include <functional>

class QEvent;
class QBackend;

typedef std::function<void(const QEvent &Event)> CallBackFunction;



class QEvent
{
public:

    QEvent();
    QEvent(QEventFD EventFD, int Events);
    virtual ~QEvent();

    inline int GetEvents() const { return m_Events; }
    inline QEventFD GetFD() const { return m_EventFD; }
    inline timeval GetTimeout() const { return m_Timeout; }
    inline void* GetExtendArg() const { return m_ExtendArg; }
    inline std::shared_ptr<QBackend> GetBackend() const { return m_Backend; }

    void SetTimeout(const timeval &Timeout) { m_Timeout = Timeout; }
    void SetCallBack(CallBackFunction CallBack, void *ExtendArg = nullptr);
    void SetBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

    void CallBack();
    bool IsValid() const;
    bool IsPersist() const;
    bool IsEqual(const QEvent &Right) const;

protected:

    int                                         m_Events;
    QEventFD                                    m_EventFD;
    timeval                                     m_Timeout;
    void                                       *m_ExtendArg;
    CallBackFunction                            m_CallBack;
    std::shared_ptr<QBackend>                   m_Backend;
};
