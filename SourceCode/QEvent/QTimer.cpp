#include "QTimer.h"
#include "Backend/QBackend.h"
#include "../QLog/QSimpleLog.h"



QTimer::QTimer()
{
}

QTimer::~QTimer()
{
}

bool QTimer::Init(QBackend &Backend)
{
    m_Channel = std::move(QChannel(-1));
    m_Channel.SetReadCallback(std::bind(&QTimer::Callback_Timeout, this, std::placeholders::_1));
    return Backend.AddEventToChannelMap(m_Channel, QEO_ADD);
}

int64_t QTimer::AddTimer(int Interval, TimerCallback Callback)
{
    if (Interval > 0)
    {
        static int64_t TimerID = 0;

        TimerNode NewNode;
        NewNode.m_TimerID = TimerID;
        NewNode.m_Callback = Callback;
        NewNode.m_Timeout = Interval;

        if (m_MinHeap.AddTimeout(NewNode))
        {
            m_TimerMap[++TimerID] = std::move(NewNode);
            return TimerID;
        }
    }

    return -1;
}

bool QTimer::DelTimer(int64_t TimerID)
{
    std::map<int64_t, TimerNode>::const_iterator it = m_TimerMap.find(TimerID);
    if (it == m_TimerMap.end())
    {
        return false;
    }

    if (!m_MinHeap.DelTimeout(it->second))
    {
        return false;
    }

    m_TimerMap.erase(it);
    return true;
}

void QTimer::Callback_Timeout(const QChannel &Channel)
{
    if (m_MinHeap.HasNode())
    {
        m_MinHeap.MinusElapsedTime(m_MinHeap.GetMinTimeout());

        while (m_MinHeap.HasNode() && m_MinHeap.Top().m_Timeout <= 0)
        {
            const TimerNode &PopNode = m_MinHeap.Pop();
            if (m_TimerMap.find(PopNode.m_TimerID) == m_TimerMap.end())
            {
                QLog::g_Log.WriteDebug("Process timeout: Can not find timer id = %d", PopNode.m_TimerID);
                continue;
            }

            m_TimerMap[PopNode.m_TimerID].m_Callback();
        }
    }
}

void QTimer::ClearTimeval(timeval &tv)
{
    tv.tv_sec = tv.tv_usec = -1;
}

bool QTimer::IsValid(const timeval &tv)
{
    return tv.tv_sec >= 0 && tv.tv_usec >= 0;
}

timeval QTimer::ConvertToTimeval(long Millisecond)
{
    timeval tv = { -1,-1 };

    if (Millisecond >= 0)
    {
        tv.tv_sec = Millisecond / 1000;
        tv.tv_usec = (Millisecond - tv.tv_sec * 1000) * 1000;
    }

    return tv;
}

long QTimer::ConvertToMillisecond(const timeval &tv)
{
    if (tv.tv_sec < 0 || tv.tv_usec < 0)
    {
        return -1;
    }

    return (tv.tv_sec * 1000) + ((tv.tv_usec + 999) / 1000);
}
