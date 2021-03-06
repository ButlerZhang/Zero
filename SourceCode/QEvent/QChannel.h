#pragma once
#include "QLibBase.h"
#include "QEventEnum.h"

#include <memory>

class QBackend;



class QChannel
{
public:

    QChannel(QEventFD EventFD);
    ~QChannel();

    bool IsValid() const;
    void HandlerEvent() const;

    void SetResultEvents(int ResultEvents);
    void SetReadCallback(EventCallback ReadCallback);
    void SetWriteCallback(EventCallback WriteCallback);
    void SetCloseCallback(EventCallback CloseCallback);
    void SetErrorCallback(EventCallback ErrorCallback);

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
    EventCallback                               m_ReadCallback;
    EventCallback                               m_WriteCallback;
    EventCallback                               m_CloseCallback;
    EventCallback                               m_ErrorCallback;

    timeval                                     m_Timeout;
    std::shared_ptr<QBackend>                   m_Backend;
};
