#pragma once
#include "../QLibBase.h"
#include "../Event/QEvent.h"
#include "../Tools/QMinHeap.h"

#include <map>



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(const QEvent &Event) = 0;
    virtual bool DelEvent(const QEvent &Event) = 0;
    virtual bool Dispatch(struct timeval *tv)  = 0;

    bool AddToMinHeap(const QEvent &Event);

    bool IsStop() const { return m_IsStop; }
    QMinHeap& GetMinHeap() { return m_MinHeap; }
    const std::string& GetBackendName() const { return m_BackendName; }

protected:

    bool                                        m_IsStop;
    std::string                                 m_BackendName;
    QMinHeap                                    m_MinHeap;
    std::map<QEventFD, QEvent>                  m_EventMap;
    static QEventFD                             m_TimeFD;
};
