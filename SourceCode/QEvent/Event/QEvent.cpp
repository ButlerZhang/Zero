#include "QEvent.h"
#include "../../QLog/QSimpleLog.h"



QEvent::QEvent()
{
    m_WatchEvents = 0;
    m_ResultEvents = 0;
    m_EventFD = -1;
    m_CallBack = nullptr;
    m_TimeOut.tv_sec = m_TimeOut.tv_usec = 0;
}

QEvent::QEvent(QEventFD EventFD, int WatchEvents)
{
    m_WatchEvents = WatchEvents;
    m_ResultEvents = 0;
    m_EventFD = EventFD;
    m_CallBack = nullptr;
    m_TimeOut.tv_sec = m_TimeOut.tv_usec = 0;
}

QEvent::QEvent(QEventFD EventFD, int WatchEvents, CallBackFunction CallBack)
{
    m_WatchEvents = WatchEvents;
    m_ResultEvents = 0;
    m_EventFD = EventFD;
    m_CallBack = CallBack;
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
        QLog::g_Log.WriteDebug("QEvent: stop process call back.");
    }
    else
    {
        QLog::g_Log.WriteDebug("QEvent: call back is nullptr, FD = %d.", GetFD());
    }
}
