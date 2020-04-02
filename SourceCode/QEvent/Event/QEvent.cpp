#include "QEvent.h"



QEvent::QEvent(QSOCKET EventFD, int Events)
{
    m_EventFD = EventFD;
    m_Events = Events;
}

QEvent::~QEvent()
{
}
