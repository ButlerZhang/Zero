#include "QChannel.h"
#include "../QLog/QSimpleLog.h"



QChannel::QChannel()
{
    m_Events = 0;
    m_ResultEvents = 0;
    m_EventFD = 0;
    m_ReadCallback = nullptr;
    m_WriteCallback = nullptr;
}

QChannel::QChannel(QEventFD EventFD)
{
    m_Events = 0;
    m_ResultEvents = 0;
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
