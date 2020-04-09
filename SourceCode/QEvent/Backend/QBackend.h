#pragma once
#include "../QLibBase.h"
#include "../Event/QEvent.h"
#include "../Tools/QMinHeap.h"

#include <map>
#include <vector>



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(const QEvent &Event);
    virtual bool DelEvent(const QEvent &Event);
    virtual bool Dispatch(struct timeval *tv)  = 0;

    bool AddToMinHeap(const QEvent &Event);
    bool IsExisted(const QEvent &Event) const;

    inline bool IsStop() const { return m_IsStop; }
    inline QMinHeap& GetMinHeap() { return m_MinHeap; }
    inline const std::string& GetBackendName() const { return m_BackendName; }

protected:

    QEventFD GetTargetFD(const QEvent &Event) const;
    void ActiveEvent(QEventFD FD, int ResultEvents);
    void WriteEventOperationLog(QEventFD MapIndex, QEventFD FD, QEventOption OP);

protected:

    bool                                        m_IsStop;
    std::string                                 m_BackendName;
    QMinHeap                                    m_MinHeap;
    std::map<QEventFD, std::vector<QEvent>>     m_EventMap;

    static QEventFD                             m_TimerFD;
};
