#pragma once
#include "../QLibBase.h"
#include "../QSignal.h"
#include "../QTimer.h"



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
    inline QTimer& GetTimer() { return m_Timer; }
    inline const std::string& GetBackendName() const { return m_BackendName; }
    inline const std::map<QEventFD, QChannel>& GetEventMap() const { return m_ChannelMap; }

    bool AddEventToChannelMap(const QChannel &Event, QEventOption OP);
    bool DelEventFromChannelMap(const QChannel &Event, QEventOption OP);

protected:

    void ActiveEvent(QEventFD FD, int ResultEvents);
    void WriteMapVectorSnapshot();

protected:

    bool                                            m_IsStop;
    std::string                                     m_BackendName;
    QEventFD                                        m_TimerFD;
    QSignal                                         m_Signal;
    QTimer                                          m_Timer;
    std::map<QEventFD, QChannel>                    m_ChannelMap;
};
