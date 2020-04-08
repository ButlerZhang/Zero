#include "QBackend.h"
#include "../../QLog/QSimpleLog.h"



QEventFD QBackend::m_TimerFD = -1;

QBackend::QBackend()
{
    m_IsStop = false;
}

QBackend::~QBackend()
{
    std::map<QEventFD, std::vector<QEvent>>::iterator it = m_EventMap.begin();
    while (it != m_EventMap.end())
    {
        it->second.clear();
    }

    m_EventMap.clear();
}

bool QBackend::AddEvent(const QEvent &Event)
{
    if (Event.GetWatchEvents() == 0)
    {
        QLog::g_Log.WriteError("%s: Add event FD = %d failed, watch events is 0.",
            m_BackendName.c_str(), Event.GetFD());
        return false;
    }

    if (IsExisted(Event))
    {
        QLog::g_Log.WriteError("%s: Add event FD = %d, watch events = %d falied, exited.",
            m_BackendName.c_str(), Event.GetFD(), Event.GetWatchEvents());
        return false;
    }

    return true;
}

bool QBackend::DelEvent(const QEvent &Event)
{
    std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(Event.GetFD());
    if (MapIt == m_EventMap.end())
    {
        QLog::g_Log.WriteError("%s: Delete event FD = %d failed, can not find FD.",
            m_BackendName.c_str(), Event.GetFD());
        return false;
    }

    std::vector<QEvent>::iterator TargetVecIt = MapIt->second.end();
    for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
    {
        if (VecIt->IsEqual(Event))
        {
            TargetVecIt = VecIt;
            break;
        }
    }

    if (TargetVecIt == MapIt->second.end())
    {
        QLog::g_Log.WriteError("%s: Delete event FD = %d failed, can not match watch events = %d.",
            m_BackendName.c_str(), Event.GetFD(), Event.GetWatchEvents());
        return false;
    }

    MapIt->second.erase(TargetVecIt);
    if (MapIt->second.empty())
    {
        m_EventMap.erase(MapIt);
    }

    return true;
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

bool QBackend::IsExisted(const QEvent &Event) const
{
    std::map<QEventFD, std::vector<QEvent>>::const_iterator MapIt= m_EventMap.find(Event.GetFD());
    if (MapIt == m_EventMap.end())
    {
        return false;
    }

    for (std::vector<QEvent>::const_iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
    {
        if (VecIt->IsEqual(Event))
        {
            return true;
        }
    }

    return false;
}

void QBackend::WriteAddLog(QEventFD AddFD) const
{
    int EventCount = 0;

    std::map<QEventFD, std::vector<QEvent>>::const_iterator it = m_EventMap.begin();
    while (it != m_EventMap.end())
    {
        EventCount += static_cast<int>(it->second.size());
        it++;
    }

    QLog::g_Log.WriteInfo("%s: Add FD = %d succeed, current event count = %d.",
        m_BackendName.c_str(), AddFD, EventCount);
}

void QBackend::WriteDelLog(QEventFD AddFD) const
{
    int EventCount = 0;

    std::map<QEventFD, std::vector<QEvent>>::const_iterator it = m_EventMap.begin();
    while (it != m_EventMap.end())
    {
        EventCount += static_cast<int>(it->second.size());
        it++;
    }

    QLog::g_Log.WriteInfo("%s: Del FD = %d succeed, current event count = %d.",
        m_BackendName.c_str(), AddFD, EventCount);
}
