#include "QChannel.h"
#include "../QLog/QSimpleLog.h"



QChannel::QChannel()
{
}

QChannel::QChannel(QEventFD EventFD)
{
    m_Events = 0;
    m_EventFD = EventFD;
    m_ReadCallback = nullptr;
    m_WriteCallback = nullptr;
}

QChannel::~QChannel()
{
}

void QChannel::SetReadCallback(EventCallback ReadCallback)
{
    m_ReadCallback = ReadCallback;
    m_Events |= QET_READ;
}

void QChannel::SetWriteCallback(EventCallback WriteCallback)
{
    m_WriteCallback = WriteCallback;
    m_Events |= QET_WRITE;
}

void QChannel::HandlerEvent()
{
    if (m_ReadCallback != nullptr)
    {
        QLog::g_Log.WriteDebug("QEvent: Start process call back.");
        m_ReadCallback(*this);
        QLog::g_Log.WriteDebug("QEvent: Stop process call back.");
    }
    else
    {
        QLog::g_Log.WriteDebug("QEvent: CallBack is nullptr, FD = %d, events = %d.",
            m_EventFD, m_Events);
    }
}
