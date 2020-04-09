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
    if (!QBackend::AddEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("select: FD = %d add read event.", Event.GetFD());
    }

    if (Event.GetEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("select: FD = %d add write event.", Event.GetFD());
    }

    if (m_HighestEventFD <= Event.GetFD())
    {
        m_HighestEventFD = Event.GetFD() + 1;
        QLog::g_Log.WriteDebug("select: Highest event FD = %d.", m_HighestEventFD);
    }

    m_EventMap[Event.GetFD()].push_back(std::move(Event));
    WriteEventOperationLog(Event.GetFD(), QEO_ADD);
    return true;
}

bool QSelect::DelEvent(const QEvent &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    FD_CLR(Event.GetFD(), &m_ReadSetIn);
    FD_CLR(Event.GetFD(), &m_WriteSetIn);

    m_HighestEventFD = 0;
    QEventOption OP = QEO_DEL;

    for (std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.begin(); MapIt != m_EventMap.end(); MapIt++)
    {
        if (MapIt->first == Event.GetFD())
        {
            OP = QEO_MOD;
            for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
            {
                int WatchEvents = VecIt->GetEvents();
                if (WatchEvents & QET_READ)
                {
                    FD_SET(VecIt->GetFD(), &m_ReadSetIn);
                }

                if (Event.GetEvents() & QET_WRITE)
                {
                    FD_SET(VecIt->GetFD(), &m_WriteSetIn);
                }
            }
        }

        if (m_HighestEventFD <= MapIt->first)
        {
            m_HighestEventFD = MapIt->first + 1;
        }
    }

    QLog::g_Log.WriteDebug("select: Highest event FD = %d.", m_HighestEventFD);
    WriteEventOperationLog(Event.GetFD(), OP);
    return true;
}

bool QSelect::Dispatch(struct timeval *tv)
{
    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    QLog::g_Log.WriteDebug("select: start...");
    int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, tv);
    QLog::g_Log.WriteDebug("select: stop, result = %d.", Result);

    if (Result < 0)
    {
        QLog::g_Log.WriteError("select error : %s", strerror(errno));
        m_IsStop = true;
        return false;
    }

    if (Result == 0)
    {
        if (m_EventMap.find(m_TimerFD) != m_EventMap.end())
        {
            m_EventMap[m_TimerFD][0].CallBack();
        }
    }
    else
    {
        for (int FD = 0; FD < m_HighestEventFD; FD++)
        {
            int ResultEvents = 0;
            if (FD_ISSET(FD, &m_ReadSetOut))
            {
                ResultEvents |= QET_READ;
            }

            if (FD_ISSET(FD, &m_WriteSetOut))
            {
                ResultEvents |= QET_WRITE;
            }

            ActiveEvent(FD, ResultEvents);
        }
    }

    return true;
}
