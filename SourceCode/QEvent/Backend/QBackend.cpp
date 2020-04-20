#include "QBackend.h"
#include "../QNetwork.h"
#include "../../QLog/QSimpleLog.h"



QBackend::QBackend()
{
    m_IsStop = false;
    m_TimerFD = -1;
}

QBackend::~QBackend()
{
    std::map<QEventFD, std::vector<QChannel>>::iterator it = m_EventMap.begin();
    while (it != m_EventMap.end())
    {
        it->second.clear();
    }

    m_EventMap.clear();
}

bool QBackend::AddEvent(const QChannel &Event)
{
    if (IsExisted(Event))
    {
        QLog::g_Log.WriteError("Add event failed, FD = %d, events = %d, it is existed.",
            Event.GetFD(), Event.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::DelEvent(const QChannel &Event)
{
    if (!IsExisted(Event))
    {
        QLog::g_Log.WriteError("Del event failed, FD = %d, events = %d, it is not existed.",
            Event.GetFD(), Event.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::ModEvent(const QChannel &Event)
{
    //TODO
    return false;
}

bool QBackend::AddEventToMapVector(const QChannel &Event, QEventOption OP)
{
    QEventFD MapKey = GetMapKey(Event);
    m_EventMap[MapKey].push_back(std::move(Event));
    WriteEventOperationLog(MapKey, Event.GetFD(), OP);
    //m_MinHeap.AddTimeout(Event, MapKey, m_EventMap[MapKey].size() - 1);
    return true;
}

bool QBackend::DelEventFromMapVector(const QChannel &Event, QEventOption OP)
{
    QEventFD MapKey = GetMapKey(Event);
    std::map<QEventFD, std::vector<QChannel>>::iterator MapIt = m_EventMap.find(MapKey);
    if (MapIt == m_EventMap.end())
    {
        QLog::g_Log.WriteError("Delete event failed, can not find FD = %d.",
            Event.GetFD());
        return false;
    }

    std::vector<QChannel>::size_type TargetIndex = -1;
    std::vector<QChannel>::iterator TargetIt = MapIt->second.end();
    for (std::vector<QChannel>::iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
    {
        //TargetIndex += 1;
        //if (VecIt->IsEqual(Event))
        //{
        //    TargetIt = VecIt;
        //    break;
        //}
    }

    if (TargetIt == MapIt->second.end())
    {
        QLog::g_Log.WriteError("Delete event failed, can not match FD = %d.",
            Event.GetFD());
        return false;
    }

    if (TargetIt->GetEvents() & QET_TIMEOUT)
    {
        m_MinHeap.DelTimeout(MapKey, TargetIndex);
    }

    MapIt->second.erase(TargetIt);
    if (MapIt->second.empty())
    {
        m_EventMap.erase(MapIt);
    }

    WriteEventOperationLog(MapKey, Event.GetFD(), OP);
    return true;
}

bool QBackend::IsExisted(const QChannel &Event) const
{
    QEventFD MapKey = GetMapKey(Event);
    std::map<QEventFD, std::vector<QChannel>>::const_iterator MapIt = m_EventMap.find(MapKey);
    if (MapIt == m_EventMap.end())
    {
        return false;
    }

    //for (std::vector<QChannel>::const_iterator VecIt = MapIt->second.begin(); VecIt != MapIt->second.end(); VecIt++)
    //{
    //    if (VecIt->IsEqual(Event))
    //    {
    //        return true;
    //    }
    //}

    return false;
}

QEventFD QBackend::GetMapKey(const QChannel &Event) const
{
    if (Event.GetEvents() & QET_TIMEOUT)
    {
        return m_TimerFD;
    }

    if (Event.GetEvents() & QET_SIGNAL)
    {
        return m_Signal.GetFD();
    }

    return Event.GetFD();
}

void QBackend::ProcessTimeout()
{
    if (m_MinHeap.HasNode())
    {
        m_MinHeap.MinusElapsedTime(m_MinHeap.GetMinTimeout());

        while (m_MinHeap.HasNode() && m_MinHeap.Top().m_Timeout <= 0)
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

            QChannel &Event = m_EventMap[PopNode.m_MapKey][PopNode.m_MapVectorIndex];

            Event.HandlerEvent();
            //if (Event.IsPersist())
            //{
            //    m_MinHeap.AddTimeout(Event, PopNode.m_MapKey, PopNode.m_MapVectorIndex);
            //}
        }
    }
}

void QBackend::ActiveEvent(QEventFD FD, int ResultEvents)
{
    QLog::g_Log.WriteDebug("Active event: FD = %d, events = %d", FD, ResultEvents);
    if (FD == m_Signal.GetFD())
    {
        m_EventMap[m_Signal.GetFD()][0].HandlerEvent();
    }
    else
    {
        for (std::vector<QChannel>::size_type Index = 0; Index < m_EventMap[FD].size(); Index++)
        {
            if (ResultEvents & QET_READ)
            {
                m_EventMap[FD][Index].HandlerEvent();
            }

            if (ResultEvents & QET_WRITE)
            {
                m_EventMap[FD][Index].HandlerEvent();
            }
        }
    }
}

void QBackend::WriteMapVectorSnapshot()
{
    QLog::g_Log.WriteDebug("==========map and vector snapshot==========");

    int MapCount = 0;
    std::map<QEventFD, std::vector<QChannel>>::const_iterator MapIt = m_EventMap.begin();
    while (MapIt != m_EventMap.end())
    {
        QLog::g_Log.WriteDebug("map index = %d, map key = %d, vector size = %d",
            MapCount++, MapIt->first, static_cast<int>(MapIt->second.size()));

        for (std::vector<QChannel>::size_type Index = 0; Index != MapIt->second.size(); Index++)
        {
            QLog::g_Log.WriteDebug("\tvec index = %ld\tFD = %d\tevents = %d",
                Index, MapIt->second[Index].GetFD(), MapIt->second[Index].GetEvents());
        }

        MapIt++;
    }

    QLog::g_Log.WriteDebug("===========================================");
}

void QBackend::WriteEventOperationLog(QEventFD MapKey, QEventFD FD, QEventOption OP)
{
    int EventCount = 0;
    int FDCount = static_cast<int>(m_EventMap.size());

    std::map<QEventFD, std::vector<QChannel>>::const_iterator it = m_EventMap.begin();
    while (it != m_EventMap.end())
    {
        EventCount += static_cast<int>((it++)->second.size());
    }

    QLog::g_Log.WriteDebug("%s: map key = %d, FD = %d, OP = %s; FD count = %d, event count = %d.",
        m_BackendName.c_str(), MapKey, FD, GetEventOptionString(OP), FDCount, EventCount);

    WriteMapVectorSnapshot();
}
