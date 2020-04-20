#pragma once
#include "QLibBase.h"
#include "QEventEnum.h"

#include <memory>
#include <functional>

class QChannel;
class QBackend;

typedef std::function<void(const QChannel &Event)> IOEventCallback;



class QChannel
{
public:

    QChannel();
    QChannel(QEventFD EventFD);
    ~QChannel();

    void HandlerEvent() const;

    void SetResultEvents(int ResultEvents);
    void SetReadCallback(IOEventCallback ReadCallback);
    void SetWriteCallback(IOEventCallback WriteCallback);

    inline int GetEvents() const { return m_Events; }
    inline int GetResultEvents() const { return m_ResultEvents; }

    inline QEventFD GetFD() const { return m_EventFD; }
    inline timeval GetTimeout() const { return m_Timeout; }
    inline std::shared_ptr<QBackend> GetBackend() const { return m_Backend; }

    //delete
    void SetTimeout(const timeval &Timeout) { m_Timeout = Timeout; }
    void SetBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

protected:

    int                                         m_Events;
    int                                         m_ResultEvents;

    QEventFD                                    m_EventFD;

    IOEventCallback                             m_ReadCallback;
    IOEventCallback                             m_WriteCallback;

    timeval                                     m_Timeout;
    std::shared_ptr<QBackend>                   m_Backend;
};