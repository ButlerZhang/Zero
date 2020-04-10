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
    if (!Event.IsValid())
    {
        QLog::g_Log.WriteError("%s: Add event FD = %d, events = %d failed, it is not valid.",
            m_BackendName.c_str(), Event.GetFD(), Event.GetEvents());
        return false;
    }

    if (IsExisted(Event))
    {
        QLog::g_Log.WriteError("%s: Add event FD = %d, events = %d falied, it is existed.",
            m_BackendName.c_str(), Event.GetFD(), Event.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::DelEvent(const QEvent &Event)
{
    QEventFD MapKey = GetMapKey(Event);
    std::map<QEventFD, std::vector<QEvent>>::iterator MapIt = m_EventMap.find(MapKey);
    if (MapIt == m_EventMap.end())
    {
        QLog::g_Log.WriteError("%s: Delete event FD = %d failed, can not find FD.",
            m_BackendName.c_str(), Event.GetFD());
        return false;
    }

    std::vector<QEvent>::iterator TargetIt = MapIt->second.end();
    for (std::vector<QEvent>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
    {
        if (VecIt->IsEqual(Event))
        {
            TargetIt = VecIt;
            break;
        }
    }

    if (TargetIt == MapIt->second.end())
    {
        QLog::g_Log.WriteError("%s: Delete event FD = %d failed, can not match.",
            m_BackendName.c_str(), Event.GetFD());
        return false;
    }

    MapIt->second.erase(TargetIt);
    if (MapIt->second.empty())
    {
        m_EventMap.erase(MapIt);
    }

    return true;
}

bool QBackend::IsExisted(const QEvent &Event) const
{
    QEventFD MapKey = GetMapKey(Event);
    std::map<QEventFD, std::vector<QEvent>>::const_iterator MapIt = m_EventMap.find(MapKey);
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

QEventFD QBackend::GetMapKey(const QEvent &Event) const
{
    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return m_TimerFD;
    }

    if (Event.GetEvents() & QET_SIGNAL)
    {
        //TODO
    }

    return Event.GetFD();
}

void QBackend::ProcessTimeout()
{
    if (m_MinHeap.HasNode())
    {
        m_MinHeap.MinusElapsedTime(m_MinHeap.GetMinTimeout());

        while (m_MinHeap.Top().m_Timeout <= 0)
        {
            const QMinHeap::HeapNode &PopNode = m_MinHeap.Pop();
            if (m_EventMap.find(PopNode.m_MapKey) == m_EventMap.end())
            {
                QLog::g_Log.WriteDebug("Process timeout: Can not find map key = %d", PopNode.m_MapKey);
                continue;
            }

            if (!(PopNode.m_MapVectorIndex >= 0 && PopNode.m_MapVectorIndex < m_EventMap[PopNode.m_MapKey].size()))
            {
                QLog::g_Log.WriteDebug("Process timeout: map key = %d has wrong vec index = %d",
                    PopNode.m_MapKey, PopNode.m_MapVectorIndex);
                continue;
            }

            QEvent &Event = m_EventMap[PopNode.m_MapKey][PopNode.m_MapVectorIndex];

            Event.CallBack();
            if (Event.IsPersist())
            {
                m_MinHeap.AddTimeout(Event, PopNode.m_MapKey, PopNode.m_MapVectorIndex);
            }
        }
    }
}

void QBackend::ActiveEvent(QEventFD FD, int ResultEvents)
{
    for (std::vector<QEvent>::size_type Index = 0; Index < m_EventMap[FD].size(); Index++)
    {
        if (ResultEvents & QET_READ)
        {
            m_EventMap[FD][Index].CallBack();
        }

        if (ResultEvents & QET_WRITE)
        {
            m_EventMap[FD][Index].CallBack();
        }
    }
}

void QBackend::WriteEventOperationLog(QEventFD MapKey, QEventFD FD, QEventOption OP)
{
    int EventCount = 0;
    int FDCount = static_cast<int>(m_EventMap.size());

    std::map<QEventFD, std::vector<QEvent>>::const_iterator it = m_EventMap.begin();
    while (it != m_EventMap.end())
    {
        EventCount += static_cast<int>((it++)->second.size());
    }

    QLog::g_Log.WriteDebug("%s: MapKey = %d, FD = %d, OP = %s, FD Count = %d, Event Count = %d.",
        m_BackendName.c_str(), MapKey, FD, GetEventOptionString(OP), FDCount, EventCount);
}
