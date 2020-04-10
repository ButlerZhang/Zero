#include "QEvent.h"
#include "../../QLog/QSimpleLog.h"



QEvent::QEvent()
{
    m_Events = 0;
    m_EventFD = -1;
    m_CallBack = nullptr;
    m_ExtendArg = nullptr;
    m_Timeout.tv_sec = m_Timeout.tv_usec = -1;
}

QEvent::QEvent(QEventFD EventFD, int Events)
{
    m_Events = Events;
    m_EventFD = EventFD;
    m_CallBack = nullptr;
    m_ExtendArg = nullptr;
    m_Timeout.tv_sec = m_Timeout.tv_usec = -1;
}

QEvent::~QEvent()
{
}

void QEvent::SetCallBack(CallBackFunction CallBack, void *ExtendArg)
{
    m_ExtendArg = ExtendArg;
    m_CallBack = std::move(CallBack);
}

void QEvent::CallBack()
{
    if (m_CallBack != nullptr)
    {
        QLog::g_Log.WriteDebug("QEvent: Start process call back.");
        m_CallBack(*this);
        QLog::g_Log.WriteDebug("QEvent: Stop process call back.");
    }
    else
    {
        QLog::g_Log.WriteDebug("QEvent: CallBack is nullptr, FD = %d, events = %d.",
            m_EventFD, m_Events);
    }
}

bool QEvent::IsValid() const
{
    if (m_Events & QET_TIMEOUT)
    {
        if (m_Timeout.tv_sec >= 0 || m_Timeout.tv_usec >= 0)
        {
            return !(m_Events & (QET_READ | QET_WRITE | QET_SIGNAL));
        }
    }

    if (m_Events & (QET_READ | QET_WRITE))
    {
        return !(m_Events & (QET_TIMEOUT | QET_SIGNAL));
    }

    if (m_Events & QET_SIGNAL)
    {
        return !(m_Events & (QET_READ | QET_WRITE | QET_TIMEOUT));
    }

    return false;
}

bool QEvent::IsPersist() const
{
    return (m_Events & QET_PERSIST) > 0;
}

bool QEvent::IsEqual(const QEvent &Right) const
{
    if ((Right.m_Events & QET_TIMEOUT) || (Right.m_Events & QET_SIGNAL))
    {
        return m_EventFD == Right.m_EventFD;
    }

    if (m_EventFD != Right.m_EventFD)
    {
        return false;
    }

    if ((m_Events & QET_READ) && (m_Events & QET_WRITE))
    {
        return (Right.m_Events & QET_READ) && (Right.m_Events & QET_WRITE);
    }

    if (m_Events & QET_READ)
    {
        return (Right.m_Events & QET_READ) && (!(Right.m_Events & QET_WRITE));
    }

    if (m_Events & QET_WRITE)
    {
        return (Right.m_Events & QET_WRITE) && (!(Right.m_Events & QET_READ));
    }

    return false;
}
