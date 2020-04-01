#pragma once
#include "../QLibBase.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

class QEvent;



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(const QEvent &Event) = 0;
    virtual bool DelEvent(const QEvent &Event) = 0;
    virtual bool Dispatch(struct timeval *tv)  = 0;

    const std::string& GetBackendName() const { return m_BackendName; }

protected:

    bool                                                                m_IsStop;
    std::string                                                         m_BackendName;
    std::map<QSOCKET, std::function<void(void)>>                        m_CallBackMap;
};
