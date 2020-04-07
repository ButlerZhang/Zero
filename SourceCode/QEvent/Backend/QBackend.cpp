#include "QBackend.h"



QEventFD QBackend::m_TimeFD = -1;

QBackend::QBackend()
{
    m_IsStop = false;
}

QBackend::~QBackend()
{
    m_EventMap.clear();
}

bool QBackend::AddToMinHeap(const QEvent &Event)
{
    if (Event.GetTimeOut().tv_sec > 0 || Event.GetTimeOut().tv_usec > 0)
    {
        m_MinHeap.AddTime(Event.GetTimeOut());
        return true;
    }

    return false;
}
