#pragma once
#include "QLibBase.h"
#include "QEventEnum.h"

#include <memory>
#include <functional>

class QChannel;
class QBackend;

typedef std::function<void(const QChannel &Event)> EventCallback;



class QChannel
{
public:

    QChannel();
    QChannel(QEventFD EventFD);
    ~QChannel();

    void HandlerEvent();

    void SetReadCallback(EventCallback ReadCallback);
    void SetWriteCallback(EventCallback WriteCallback);



    inline int GetEvents() const { return m_Events; }
    inline QEventFD GetFD() const { return m_EventFD; }
    inline timeval GetTimeout() const { return m_Timeout; }
    inline std::shared_ptr<QBackend> GetBackend() const { return m_Backend; }

    void SetTimeout(const timeval &Timeout) { m_Timeout = Timeout; }
    void SetBackend(const std::shared_ptr<QBackend> &Backend) { m_Backend = Backend; }

protected:

    int                                         m_Events;
    QEventFD                                    m_EventFD;
    EventCallback                               m_ReadCallback;
    EventCallback                               m_WriteCallback;


    timeval                                     m_Timeout;
    std::shared_ptr<QBackend>                   m_Backend;
};
