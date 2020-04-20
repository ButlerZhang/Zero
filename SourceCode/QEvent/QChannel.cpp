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

void QChannel::SetResultEvents(int ResultEvents)
{
    m_ResultEvents = ResultEvents;
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
    QLog::g_Log.WriteDebug("QChannel: FD = %d, result events = %d.",
        m_EventFD, m_Events);

    if (m_ResultEvents & QET_READ)
    {
        if (m_ReadCallback != nullptr)
        {
            m_ReadCallback(*this);
        }
        else
        {
            QLog::g_Log.WriteDebug("QChannel: FD = %d read callback is nullptr.",
                m_EventFD);
        }
    }

    if (m_ResultEvents & QET_WRITE)
    {
        if (m_WriteCallback != nullptr)
        {
            m_WriteCallback(*this);
        }
        else
        {
            QLog::g_Log.WriteDebug("QChannel: FD = %d write callback is nullptr.",
                m_EventFD);
        }
    }
}
