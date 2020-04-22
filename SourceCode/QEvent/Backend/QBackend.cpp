#include "QBackend.h"
#include "../QLog.h"
#include "../QNetwork.h"



QBackend::QBackend(QEventLoop &EventLoop) : m_EventLoop(EventLoop)
{
}

QBackend::~QBackend()
{
    m_ChannelMap.clear();
}

bool QBackend::AddEvent(const QChannel &Channel)
{
    if (!Channel.IsValid())
    {
        g_Log.WriteError("Add channel failed, FD = %d, events = %d is not valid.",
            Channel.GetFD(), Channel.GetEvents());
        return false;
    }

    if (m_ChannelMap.find(Channel.GetFD()) != m_ChannelMap.end())
    {
        g_Log.WriteError("Add channel failed, FD = %d, events = %d, it is existed.",
            Channel.GetFD(), Channel.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::DelEvent(const QChannel &Channel)
{
    if (m_ChannelMap.find(Channel.GetFD()) == m_ChannelMap.end())
    {
        g_Log.WriteError("Del channel failed, FD = %d, events = %d, it is not existed.",
            Channel.GetFD(), Channel.GetEvents());
        return false;
    }

    return true;
}

bool QBackend::ModEvent(const QChannel &Channel)
{
    if (m_ChannelMap.find(Channel.GetFD()) == m_ChannelMap.end())
    {
        g_Log.WriteError("Mod channel failed, FD = %d, events = %d, it is not existed.",
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
    return true;
}

bool QBackend::DelEventFromChannelMap(const QChannel &Channel, QEventOption OP)
{
    std::map<QEventFD, QChannel>::iterator it = m_ChannelMap.find(Channel.GetFD());
    if (it == m_ChannelMap.end())
    {
        g_Log.WriteError("Delete channel failed, can not find FD = %d.",
            Channel.GetFD());
        return false;
    }

    m_ChannelMap.erase(it);
    WriteMapVectorSnapshot();
    return true;
}

void QBackend::ActiveEvent(QEventFD FD, int ResultEvents)
{
    g_Log.WriteDebug("Active event: FD = %d, events = %d", FD, ResultEvents);
    m_ChannelMap[FD].SetResultEvents(ResultEvents);
    m_ChannelMap[FD].HandlerEvent();
}

void QBackend::WriteMapVectorSnapshot()
{
    g_Log.WriteDebug("==========channel map snapshot==========");

    int MapCount = 0;
    std::map<QEventFD, QChannel>::const_iterator it = m_ChannelMap.begin();
    while (it != m_ChannelMap.end())
    {
        g_Log.WriteDebug("map index = %d, FD = %d, events = %d",
            MapCount++, it->second.GetFD(), it->second.GetEvents());

        it++;
    }

    g_Log.WriteDebug("========================================");
}
