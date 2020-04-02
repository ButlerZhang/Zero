#include "QEvent.h"



QEvent::QEvent(QSOCKET EventFD, int Events)
{
    m_EventFD = EventFD;
    m_Events = Events;
    m_CallBack = nullptr;
}

QEvent::~QEvent()
{
}

void QEvent::CallBack()
{
    if (m_CallBack != nullptr)
    {
        m_CallBack(*this);
    }
}
