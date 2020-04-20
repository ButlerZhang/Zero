#pragma once
#include "QLibBase.h"
#include "QChannel.h"
#include "QMinHeap.h"
#include <map>


class QTimer
{
public:

    QTimer();
    ~QTimer();

    bool Init(QBackend &Backend);

    QEventFD GetFD() const { return m_Channel.GetFD(); }
    long GetMinTimeout() const { return m_MinHeap.GetMinTimeout(); }

    int64_t AddTimer(int Interval, TimerCallback Callback);
    bool DelTimer(int64_t TimerID);

private:

    void Callback_Timeout(const QChannel &Channel);

private:

    QChannel                                        m_Channel;
    QMinHeap                                        m_MinHeap;
    std::map<int64_t, TimerCallback>                m_TimerMap;
};
