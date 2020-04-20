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
        int64_t TimerID = 0;
        m_TimerMap[TimerID] = std::move(Callback);
        return true;
    }

    return false;
}

bool QTimer::DelTimer(int64_t TimerID)
{
    std::map<int64_t, TimerCallback>::const_iterator it = m_TimerMap.find(TimerID);
    if (it == m_TimerMap.end())
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
            const QMinHeap::HeapNode &PopNode = m_MinHeap.Pop();
            if (m_TimerMap.find(PopNode.m_MapKey) == m_TimerMap.end())
            {
                QLog::g_Log.WriteDebug("Process timeout: Can not find map key = %d", PopNode.m_MapKey);
                continue;
            }

            m_TimerMap[PopNode.m_MapKey]();
        }
    }
}
