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
    m_ChannelMap.clear();
}

bool QBackend::AddEvent(const QChannel &Channel)
{
    if (m_ChannelMap.find(Channel.GetFD()) != m_ChannelMap.end())
    {
        QLog::g_Log.WriteError("Add channel failed, FD = %d, events = %d, it is existed.",
            Channel.GetFD(), Channel.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::DelEvent(const QChannel &Channel)
{
    if (m_ChannelMap.find(Channel.GetFD()) == m_ChannelMap.end())
    {
        QLog::g_Log.WriteError("Del channel failed, FD = %d, events = %d, it is not existed.",
            Channel.GetFD(), Channel.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::ModEvent(const QChannel &Channel)
{
    if (m_ChannelMap.find(Channel.GetFD()) == m_ChannelMap.end())
    {
        QLog::g_Log.WriteError("Mod channel failed, FD = %d, events = %d, it is not existed.",
            Channel.GetFD(), Channel.GetEvents());
        return false;
    }

    //TODO
    return true;
}

bool QBackend::AddEventToChannelMap(const QChannel &Channel, QEventOption OP)
{
    m_ChannelMap[Channel.GetFD()] = std::move(Channel);
    WriteMapVectorSnapshot();

    //m_MinHeap.AddTimeout(Event, MapKey, m_ChannelMap[MapKey].size() - 1);
    return true;
}

bool QBackend::DelEventFromChannelMap(const QChannel &Channel, QEventOption OP)
{
    std::map<QEventFD, QChannel>::iterator it = m_ChannelMap.find(Channel.GetFD());
    if (it == m_ChannelMap.end())
    {
        QLog::g_Log.WriteError("Delete channel failed, can not find FD = %d.",
            Channel.GetFD());
        return false;
    }

    //if (TargetIt->GetEvents() & QET_TIMEOUT)
    //{
    //    m_MinHeap.DelTimeout(MapKey, TargetIndex);
    //}

    m_ChannelMap.erase(it);
    WriteMapVectorSnapshot();
    return true;
}

void QBackend::ProcessTimeout()
{
    //if (m_MinHeap.HasNode())
    //{
    //    m_MinHeap.MinusElapsedTime(m_MinHeap.GetMinTimeout());

    //    while (m_MinHeap.HasNode() && m_MinHeap.Top().m_Timeout <= 0)
    //    {
    //        const QMinHeap::HeapNode &PopNode = m_MinHeap.Pop();
    //        if (m_ChannelMap.find(PopNode.m_MapKey) == m_ChannelMap.end())
    //        {
    //            QLog::g_Log.WriteDebug("Process timeout: Can not find map key = %d", PopNode.m_MapKey);
    //            continue;
    //        }

    //        if (!(PopNode.m_MapVectorIndex >= 0 && PopNode.m_MapVectorIndex < m_ChannelMap[PopNode.m_MapKey].size()))
    //        {
    //            QLog::g_Log.WriteDebug("Process timeout: map key = %d has wrong vec index = %d",
    //                PopNode.m_MapKey, PopNode.m_MapVectorIndex);
    //            continue;
    //        }

    //        QChannel &Event = m_ChannelMap[PopNode.m_MapKey][PopNode.m_MapVectorIndex];

    //        Event.HandlerEvent();
    //        //if (Event.IsPersist())
    //        //{
    //        //    m_MinHeap.AddTimeout(Event, PopNode.m_MapKey, PopNode.m_MapVectorIndex);
    //        //}
    //    }
    //}
}

void QBackend::ActiveEvent(QEventFD FD, int ResultEvents)
{
    QLog::g_Log.WriteDebug("Active event: FD = %d, events = %d", FD, ResultEvents);
    m_ChannelMap[FD].SetResultEvents(ResultEvents);
    m_ChannelMap[FD].HandlerEvent();
}

void QBackend::WriteMapVectorSnapshot()
{
    QLog::g_Log.WriteDebug("==========channel map snapshot==========");

    int MapCount = 0;
    std::map<QEventFD, QChannel>::const_iterator it = m_ChannelMap.begin();
    while (it != m_ChannelMap.end())
    {
        QLog::g_Log.WriteDebug("map index = %d, map key = %d, FD = %d, events = %d",
            MapCount++, it->first, it->second.GetFD(), it->second.GetEvents());

        it++;
    }

    QLog::g_Log.WriteDebug("========================================");
}
