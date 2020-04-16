#include "QSelect.h"
#include "../QTime.h"
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

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return AddEventToMapVector(Event, QEO_ADD);
    }

    if (Event.GetEvents() & QET_SIGNAL)
    {
        return m_Signal.Register(Event) && AddEventToMapVector(Event, QEO_ADD);
    }

    if (Event.GetEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
        QLog::g_Log.WriteDebug("select: FD = %d add read event.",
            Event.GetFD());
    }

    if (Event.GetEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
        QLog::g_Log.WriteDebug("select: FD = %d add write event.",
            Event.GetFD());
    }

    if (m_HighestEventFD <= Event.GetFD())
    {
        m_HighestEventFD = Event.GetFD() + 1;
        QLog::g_Log.WriteDebug("select: Highest event FD = %d after added.",
            m_HighestEventFD);
    }

    return AddEventToMapVector(Event, QEO_ADD);
}

bool QSelect::DelEvent(const QEvent &Event)
{
    if (!QBackend::DelEvent(Event))
    {
        return false;
    }

    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return DelEventFromMapVector(Event, QEO_DEL);
    }

    FD_CLR(Event.GetFD(), &m_ReadSetIn);
    FD_CLR(Event.GetFD(), &m_WriteSetIn);

    for (std::vector<QEvent>::iterator VecIt = m_EventMap[Event.GetFD()].begin(); VecIt != m_EventMap[Event.GetFD()].end(); VecIt++)
    {
        if (!VecIt->IsEqual(Event))
        {
            if (VecIt->GetEvents() & QET_READ)
            {
                FD_SET(VecIt->GetFD(), &m_ReadSetIn);
            }

            if (VecIt->GetEvents() & QET_WRITE)
            {
                FD_SET(VecIt->GetFD(), &m_WriteSetIn);
            }
        }
    }

    for (int FD = m_HighestEventFD - 1; FD >= 0; FD--)
    {
        if (FD_ISSET(FD, &m_ReadSetIn) || FD_ISSET(FD, &m_WriteSetIn))
        {
            break;
        }

        --m_HighestEventFD;
    }

    QLog::g_Log.WriteDebug("select: Highest event FD = %d after deleted.",
        m_HighestEventFD);

    return DelEventFromMapVector(Event, QEO_DEL);
}

bool QSelect::Dispatch(timeval &tv)
{
    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    QLog::g_Log.WriteDebug("select: start...");
    timeval *TempTimeout = QTime::IsValid(tv) ? &tv : NULL;
    int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, TempTimeout);
    QLog::g_Log.WriteDebug("select: stop, result = %d.", Result);

    if (Result < 0)
    {
        if (errno != EINTR)
        {
            QLog::g_Log.WriteError("select error : %s", strerror(errno));
            m_IsStop = true;
            return false;
        }
    }

    if (Result == 0)
    {
        ProcessTimeout();
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

            if (ResultEvents > 0)
            {
                ActiveEvent(FD, ResultEvents);
            }
        }
    }

    return true;
}
