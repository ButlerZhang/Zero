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

    virtual bool AddEvent(const QChannel &Channel);
    virtual bool DelEvent(const QChannel &Channel);
    virtual bool ModEvent(const QChannel &Channel);
    virtual bool Dispatch(timeval &tv)  = 0;

    inline bool IsStop() const { return m_IsStop; }
    inline QSignal& GetSignal() { return m_Signal; }
    inline QMinHeap& GetMinHeap() { return m_MinHeap; }
    inline const std::string& GetBackendName() const { return m_BackendName; }
    inline const std::map<QEventFD, QChannel>& GetEventMap() const { return m_ChannelMap; }

protected:

    bool AddEventToChannelMap(const QChannel &Event, QEventOption OP);
    bool DelEventFromChannelMap(const QChannel &Event, QEventOption OP);

    void ProcessTimeout();
    void ActiveEvent(QEventFD FD, int ResultEvents);

    void WriteMapVectorSnapshot();

protected:

    bool                                            m_IsStop;
    std::string                                     m_BackendName;
    QEventFD                                        m_TimerFD;
    QMinHeap                                        m_MinHeap;
    QSignal                                         m_Signal;
    std::map<QEventFD, QChannel>                    m_ChannelMap;
};
