#include "QChannel.h"
#include "QLog.h"



QChannel::QChannel(QEventFD EventFD)
{
    m_Events = 0;
    m_ResultEvents = 0;
    m_EventFD = EventFD;
    m_ReadCallback = nullptr;
    m_WriteCallback = nullptr;
    m_CloseCallback = nullptr;
    m_ErrorCallback = nullptr;
}

QChannel::~QChannel()
{
}

bool QChannel::IsValid() const
{
    return m_EventFD >= 0 && m_Events > 0;
}

void QChannel::HandlerEvent() const
{
    if (m_ResultEvents & QET_READ)
    {
        if (m_ReadCallback != nullptr)
        {
            m_ReadCallback();
        }
        else
        {
            g_Log.WriteDebug("QChannel: FD = %d read callback is nullptr.",
                m_EventFD);
        }
    }

    if (m_ResultEvents & QET_WRITE)
    {
        if (m_WriteCallback != nullptr)
        {
            m_WriteCallback();
        }
        else
        {
            g_Log.WriteDebug("QChannel: FD = %d write callback is nullptr.",
                m_EventFD);
        }
    }
}

void QChannel::SetResultEvents(int ResultEvents)
{
    m_ResultEvents = ResultEvents;
}

void QChannel::SetReadCallback(EventCallback ReadCallback)
{
    m_ReadCallback = ReadCallback;
    if (m_ReadCallback != nullptr)
    {
        m_Events |= QET_READ;
    }
    else
    {
        m_Events &= ~QET_READ;
    }
}

void QChannel::SetWriteCallback(EventCallback WriteCallback)
{
    m_WriteCallback = WriteCallback;
    if (m_WriteCallback != nullptr)
    {
        m_Events |= QET_WRITE;
    }
    else
    {
        m_Events &= ~QET_WRITE;
    }
}

void QChannel::SetCloseCallback(EventCallback CloseCallback)
{
    m_CloseCallback = CloseCallback;
}

void QChannel::SetErrorCallback(EventCallback ErrorCallback)
{
    m_ErrorCallback = ErrorCallback;
}
