#include "QSelect.h"
#include "../../QLog/QSimpleLog.h"

#include <sys/select.h>
#include <string.h>



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
    if (Event.GetEvents() == 0)
    {
        return false;
    }

    if (Event.GetEvents() & QET_READ)
    {
        FD_SET(Event.GetFD(), &m_ReadSetIn);
    }

    if (Event.GetEvents() & QET_WRITE)
    {
        FD_SET(Event.GetFD(), &m_WriteSetIn);
    }

    if (m_HighestEventFD <= Event.GetFD())
    {
        m_HighestEventFD = Event.GetFD() + 1;
    }

    m_EventMap[Event.GetFD()] = std::move(Event);
    QLog::g_Log.WriteInfo("Select : Add new fd = %d, HighestFD = %d", Event.GetFD(), m_HighestEventFD);
    return true;
}

bool QSelect::DelEvent(const QEvent &Event)
{
    if (Event.GetEvents() == 0)
    {
        return false;
    }

    if (Event.GetEvents() & QET_READ)
    {
        FD_CLR(Event.GetFD(), &m_ReadSetIn);
    }

    if (Event.GetEvents() & QET_WRITE)
    {
        FD_CLR(Event.GetFD(), &m_WriteSetIn);
    }

    return true;
}

bool QSelect::Dispatch(timeval *tv)
{
    while (!m_IsStop)
    {
        memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
        memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

        QLog::g_Log.WriteInfo("Start select...");
        int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, NULL);
        QLog::g_Log.WriteInfo("Stop select...");

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
