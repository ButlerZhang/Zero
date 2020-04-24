#include "QSelect.h"
#include "../QEventLoop.h"
#include "../QTimer.h"
#include "../QLog.h"

#include <cstring>                     //strerror



QSelect::QSelect(QEventLoop &EventLoop) : QBackend(EventLoop)
{
    m_HighestEventFD = 0;
    m_BackendName = "select";

    FD_ZERO(&m_ReadSetIn);
    FD_ZERO(&m_WriteSetIn);
}

QSelect::~QSelect()
{
}

bool QSelect::AddEvent(const std::shared_ptr<QChannel> &Channel)
{
    if (!QBackend::AddEvent(Channel))
    {
        return false;
    }

    if (Channel->GetEvents() & QET_READ)
    {
        FD_SET(Channel->GetFD(), &m_ReadSetIn);
        g_Log.WriteDebug("select: FD = %d add read event.",
            Channel->GetFD());
    }

    if (Channel->GetEvents() & QET_WRITE)
    {
        FD_SET(Channel->GetFD(), &m_WriteSetIn);
        g_Log.WriteDebug("select: FD = %d add write event.",
            Channel->GetFD());
    }

    if (m_HighestEventFD <= Channel->GetFD())
    {
        m_HighestEventFD = Channel->GetFD() + 1;
        g_Log.WriteDebug("select: Highest event FD = %d after added.",
            m_HighestEventFD);
    }

    return AddEventToChannelMap(Channel, QEO_ADD);
}

bool QSelect::DelEvent(const std::shared_ptr<QChannel> &Channel)
{
    if (!QBackend::DelEvent(Channel))
    {
        return false;
    }

    FD_CLR(Channel->GetFD(), &m_ReadSetIn);
    FD_CLR(Channel->GetFD(), &m_WriteSetIn);

    for (int FD = m_HighestEventFD - 1; FD >= 0; FD--)
    {
        if (FD_ISSET(FD, &m_ReadSetIn) || FD_ISSET(FD, &m_WriteSetIn))
        {
            break;
        }

        --m_HighestEventFD;
    }

    g_Log.WriteDebug("select: Highest event FD = %d after deleted.",
        m_HighestEventFD);

    return DelEventFromChannelMap(Channel, QEO_DEL);
}

bool QSelect::Dispatch(timeval &tv)
{
    memcpy(&m_ReadSetOut, &m_ReadSetIn, sizeof(m_ReadSetIn));
    memcpy(&m_WriteSetOut, &m_WriteSetIn, sizeof(m_WriteSetIn));

    g_Log.WriteDebug("select: start...");
    timeval *TempTimeout = QTimer::IsValid(tv) ? &tv : NULL;
    int Result = select(m_HighestEventFD, &m_ReadSetOut, &m_WriteSetOut, NULL, TempTimeout);
    g_Log.WriteDebug("select: stop, result = %d.", Result);

    if (Result < 0)
    {
        if (errno != EINTR)
        {
            g_Log.WriteError("select error : %s", strerror(errno));
            m_EventLoop.StopLoop();
            return false;
        }
    }

    if (Result == 0)
    {
        ActiveEvent(m_EventLoop.GetTimer()->GetFD(), 0);
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
