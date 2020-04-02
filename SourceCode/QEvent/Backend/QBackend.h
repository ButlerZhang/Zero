#pragma once
#include "../QLibBase.h"
#include "../Event/QEvent.h"

#include <map>
#include <string>
#include <functional>

typedef std::function<void(const QEvent &Event)> CallBackFunction;



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(const QEvent &Event, CallBackFunction CallBack) = 0;
    virtual bool DelEvent(const QEvent &Event) = 0;
    virtual bool Dispatch(struct timeval *tv)  = 0;

    const std::string& GetBackendName() const { return m_BackendName; }

protected:

    bool                                        m_IsStop;
    std::string                                 m_BackendName;
    std::map<QSOCKET, QEvent>                   m_EventMap;
    std::map<QSOCKET, CallBackFunction>         m_CallBackMap;
};
