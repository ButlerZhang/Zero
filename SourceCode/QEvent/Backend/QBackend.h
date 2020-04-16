#pragma once
#include "../QLibBase.h"
#include "../QMinHeap.h"

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

    bool InitSignal();

    inline bool IsStop() const { return m_IsStop; }
    inline QMinHeap& GetMinHeap() { return m_MinHeap; }
    inline const std::string& GetBackendName() const { return m_BackendName; }

protected:

    bool AddEventToMapVector(const QEvent &Event, QEventOption OP);
    bool DelEventFromMapVector(const QEvent &Event, QEventOption OP);

    bool AddSignal(const QEvent &Event);

    bool IsExisted(const QEvent &Event) const;
    QEventFD GetMapKey(const QEvent &Event) const;

    void ProcessTimeout();
    void ActiveEvent(QEventFD FD, int ResultEvents);

    void WriteMapVectorSnapshot();
    void WriteEventOperationLog(QEventFD MapKey, QEventFD FD, QEventOption OP);

    void Callback_Signal(const QEvent &Event);
    static void Callback_SignalHandler(QEventFD Signal);

protected:

    bool                                            m_IsStop;
    std::string                                     m_BackendName;
    QMinHeap                                        m_MinHeap;
    std::map<QEventFD, std::vector<QEvent>>         m_EventMap;

    static QEventFD                                 m_TimerFD;
    static QEventFD                                 m_SignalReadFD;
    static QEventFD                                 m_SignalWriteFD;
};
