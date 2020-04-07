#include "QSelect.h"
#include "../../QLog/QSimpleLog.h"
#include <string.h>                     //strerror



QSelect::QSelect()
{
    m_HighestEventFD = 0;
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
        QLog::g_Log.WriteDebug("Select: FD = %d add read event.", Event.GetFD());
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("Select: FD = %d add write event.", Event.GetFD());
    }

    if (m_HighestEventFD <= Event.GetFD())
    {
        m_HighestEventFD = Event.GetFD() + 1;
    }

    m_EventMap[Event.GetFD()] = std::move(Event);
    QLog::g_Log.WriteInfo("Select: FD = %d add successed, HighestFD = %d, event count = %d.",
        Event.GetFD(),
        m_HighestEventFD,
        static_cast<int>(m_EventMap.size()));

    return true;
}

bool QSelect::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, QEvent>::const_iterator it = m_EventMap.find(Event.GetFD());
    if (it == m_EventMap.end())
    {
        QLog::g_Log.WriteError("Select: Can not find FD = %d.", Event.GetFD());
        return false;
    }

    if (Event.GetWatchEvents() & QET_READ)
    {
        FD_CLR(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("Select: FD = %d clear read event.", Event.GetFD());
    }

    if (Event.GetWatchEvents() & QET_WRITE)
    {
        FD_CLR(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("Select: FD = %d clear write event.", Event.GetFD());
    }

    m_EventMap.erase(it);
    QLog::g_Log.WriteInfo("Select: FD = %d delete successed, HighestFD = %d, event count = %d.",
        Event.GetFD(),
        m_HighestEventFD,
        static_cast<int>(m_EventMap.size()));

    return true;
}

bool QSelect::Dispatch(struct timeval *tv)
{
    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    QLog::g_Log.WriteDebug("Select: start...");
    int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
    QLog::g_Log.WriteDebug("Select: stop, result = %d.", Result);

    if (Result < 0)
    {
        QLog::g_Log.WriteError("Select error : %s", strerror(errno));
        m_IsStop = true;
        return false;
    }

    if (Result == 0)
    {
        if (m_EventMap.find(m_TimeFD) != m_EventMap.end())
        {
            m_EventMap[m_TimeFD].CallBack();
        }
    }
    else
    {
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
