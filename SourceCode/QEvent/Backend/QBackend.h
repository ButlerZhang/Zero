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

    virtual bool AddEvent(const QChannel &Event);
    virtual bool DelEvent(const QChannel &Event);
    virtual bool ModEvent(const QChannel &Event);
    virtual bool Dispatch(timeval &tv)  = 0;

    inline bool IsStop() const { return m_IsStop; }
    inline QSignal& GetSignal() { return m_Signal; }
    inline QMinHeap& GetMinHeap() { return m_MinHeap; }
    inline const std::string& GetBackendName() const { return m_BackendName; }
    inline const std::map<QEventFD, std::vector<QChannel>>& GetEventMap() const { return m_EventMap; }

protected:

    bool AddEventToMapVector(const QChannel &Event, QEventOption OP);
    bool DelEventFromMapVector(const QChannel &Event, QEventOption OP);

    bool IsExisted(const QChannel &Event) const;
    QEventFD GetMapKey(const QChannel &Event) const;

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
    std::map<QEventFD, std::vector<QChannel>>         m_EventMap;
};
