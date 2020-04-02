#include "QSelect.h"
#include "../../QLog/QSimpleLog.h"
#include <string.h>                     //strerror



QSelect::QSelect()
{
    m_HighestEventFD = -1;
    m_BackendName = "select";

    FD_ZERO(&m_ReadSetIn);
    FD_ZERO(&m_WriteSetIn);
}

QSelect::~QSelect()
{
}

bool QSelect::AddEvent(const QEvent &Event)
{
    if (Event.GetWatchEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
    }

    if (m_HighestEventFD <= Event.GetFD())
    {
        m_HighestEventFD = Event.GetFD() + 1;
    }

    m_EventMap[Event.GetFD()] = std::move(Event);
    QLog::g_Log.WriteInfo("Select : Add new EventFD = %d, HighestFD = %d", Event.GetFD(), m_HighestEventFD);
    return true;
}

bool QSelect::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
        return false;
    }

    if (Event.GetWatchEvents() & QET_READ)
    {
        FD_CLR(Event.GetFD(), &m_ReadSetIn);
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        FD_CLR(Event.GetFD(), &m_WriteSetIn);
    }

    m_EventMap.erase(it);
    QLog::g_Log.WriteInfo("Select : Delete EventFD = %d", Event.GetFD());
    return true;
}

bool QSelect::Dispatch(timeval *tv)
{
    while (!m_IsStop)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        QLog::g_Log.WriteDebug("Start select...");
        int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
        QLog::g_Log.WriteDebug("Stop select...");

        if (Result <= 0)
        {
            QLog::g_Log.WriteError("Select error : %s", strerror(errno));
            return false;
        }

        for (int FD = 0; FD < m_HighestEventFD; FD++)
        {
            if (FD_ISSET(FD, &m_ReadSetOut))
            {
                m_EventMap[FD].CallBack();
            }

            if (FD_ISSET(FD, &m_WriteSetOut))
            {
                m_EventMap[FD].CallBack();
            }
        }
    }

    return true;
}
