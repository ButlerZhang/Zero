#pragma once
#include "QLibBase.h"
#include "QChannel.h"
#include "QMinHeap.h"
#include <map>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif // _WIN32


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

    static void ClearTimeval(timeval &tv);
    static bool IsValid(const timeval &tv);

    static timeval ConvertToTimeval(long Millisecond);
    static long ConvertToMillisecond(const timeval &tv);

private:

    void Callback_Timeout(const QChannel &Channel);

private:

    QChannel                                        m_Channel;
    QMinHeap                                        m_MinHeap;
    std::map<int64_t, TimerCallback>                m_TimerMap;
};
