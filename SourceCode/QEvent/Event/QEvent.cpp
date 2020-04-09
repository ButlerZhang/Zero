#include "QEvent.h"
#include "../../QLog/QSimpleLog.h"



QEvent::QEvent()
{
    m_ExtendArg = NULL;
    m_Events = 0;
    m_EventFD = -1;
    m_CallBack = nullptr;
    m_TimeOut.tv_sec = m_TimeOut.tv_usec = 0;
}

QEvent::QEvent(QEventFD EventFD, int WatchEvents)
{
    m_ExtendArg = NULL;
    m_Events = WatchEvents;
    m_EventFD = EventFD;
    m_CallBack = nullptr;
    m_TimeOut.tv_sec = m_TimeOut.tv_usec = 0;
}

QEvent::~QEvent()
{
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
        QLog::g_Log.WriteDebug("QEvent: call back is nullptr, FD = %d.", GetFD());
    }
}

bool QEvent::IsEqual(const QEvent &Right) const
{
    return m_EventFD == Right.m_EventFD && m_Events == Right.m_Events;
}

void QEvent::SetCallBack(CallBackFunction CallBack, void *ExtendArg)
{
    m_CallBack = std::move(CallBack);
    if (ExtendArg != NULL)
    {
        m_ExtendArg = ExtendArg;
    }
}
