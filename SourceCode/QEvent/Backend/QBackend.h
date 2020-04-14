#pragma once
#include "../QLibBase.h"
#include "../Tools/QMinHeap.h"

#include <map>



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(const QEvent &Event);
    virtual bool DelEvent(const QEvent &Event);
    virtual bool Dispatch(timeval &tv)  = 0;

    inline bool IsStop() const { return m_IsStop; }
    inline QMinHeap& GetMinHeap() { return m_MinHeap; }
    inline const std::string& GetBackendName() const { return m_BackendName; }

protected:

    bool AddTimeoutEvent(const QEvent &Event);
    bool DelTimeoutEvent(const QEvent &Event);

    bool DelEventFromMapVector(const QEvent &Event);
    bool AddEventToMapVector(const QEvent &Event, QEventOption OP);


    bool IsExisted(const QEvent &Event) const;
    QEventFD GetMapKey(const QEvent &Event) const;

    void ProcessTimeout();
    void ActiveEvent(QEventFD FD, int ResultEvents);
    void WriteEventOperationLog(QEventFD MapKey, QEventFD FD, QEventOption OP);

protected:

    bool                                            m_IsStop;
    std::string                                     m_BackendName;
    QMinHeap                                        m_MinHeap;
    std::map<QEventFD, std::vector<QEvent>>         m_EventMap;

    static QEventFD                                 m_TimerFD;
};
