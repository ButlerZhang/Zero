#pragma once
#include "../QLibBase.h"
#include "../QMinHeap.h"
#include "../QSignal.h"

#include <map>



class QBackend
{
public:

    QBackend();
    virtual ~QBackend();

    virtual bool AddEvent(const QEvent &Event);
    virtual bool DelEvent(const QEvent &Event);
    virtual bool ModEvent(const QEvent &Event);
    virtual bool Dispatch(timeval &tv)  = 0;

    inline bool IsStop() const { return m_IsStop; }
    inline QSignal& GetSignal() { return m_Signal; }
    inline QMinHeap& GetMinHeap() { return m_MinHeap; }
    inline const std::string& GetBackendName() const { return m_BackendName; }
    inline const std::map<QEventFD, std::vector<QEvent>>& GetEventMap() const { return m_EventMap; }

protected:

    bool AddEventToMapVector(const QEvent &Event, QEventOption OP);
    bool DelEventFromMapVector(const QEvent &Event, QEventOption OP);

    bool IsExisted(const QEvent &Event) const;
    QEventFD GetMapKey(const QEvent &Event) const;

    void ProcessTimeout();
    void ActiveEvent(QEventFD FD, int ResultEvents);

    void WriteMapVectorSnapshot();
    void WriteEventOperationLog(QEventFD MapKey, QEventFD FD, QEventOption OP);

protected:

    bool                                            m_IsStop;
    std::string                                     m_BackendName;
    QEventFD                                        m_TimerFD;
    QMinHeap                                        m_MinHeap;
    QSignal                                         m_Signal;
    std::map<QEventFD, std::vector<QEvent>>         m_EventMap;
};
