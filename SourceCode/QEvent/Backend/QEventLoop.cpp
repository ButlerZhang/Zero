#include "QEventLoop.h"
#include "QBackend.h"
#include "../QLog.h"
#include "../QTimer.h"
#include "../QSignal.h"

#ifdef _WIN32
#include "QWin32Select.h"
#else
#include "QSelect.h"
#include "QPoll.h"
#include "QEpoll.h"
#endif



QEventLoop::QEventLoop()
{
    m_IsStop = false;
}

QEventLoop::~QEventLoop()
{
}

bool QEventLoop::Init()
{
#ifdef _WIN32
    m_Backend = std::make_shared<QWin32Select>(*this);
#else
    m_Backend = std::make_shared<QSelect>(*this);
    //m_Backend = std::make_shared<QPoll>(*this);
    //m_Backend = std::make_shared<QEpoll>(*this);
#endif

    g_Log.WriteInfo("Enable backend : %s",
        m_Backend->GetBackendName().c_str());

    m_Timer = std::make_shared<QTimer>();
    //if (m_Timer == nullptr || !m_Timer->Init(*m_Backend))
    //{
    //    g_Log.WriteError("Timer init failed.");
    //    return false;
    //}

    //m_Signal = std::make_shared<QSignal>();
    //if (m_Signal == nullptr || !m_Signal->Init(*m_Backend))
    //{
    //    g_Log.WriteError("Signal init failed.");
    //    return false;
    //}

    return true;
}

bool QEventLoop::StopLoop()
{
    m_IsStop = true;
    return true;
}

bool QEventLoop::AddEvent(const std::shared_ptr<QChannel> &Event)
{
    return m_Backend->AddEvent(Event);
}

bool QEventLoop::DelEvent(const std::shared_ptr<QChannel> &Event)
{
    return m_Backend->DelEvent(Event);
}

bool QEventLoop::ModEvent(const std::shared_ptr<QChannel> &Event)
{
    return m_Backend->ModEvent(Event);
}

bool QEventLoop::AddSignal(int Signal, SignalCallback Callback)
{
    return m_Signal->AddSignal(Signal, Callback);
}

bool QEventLoop::DelSignal(int Signal)
{
    return m_Signal->DelSignal(Signal);
}

int64_t QEventLoop::AddTimer(int Interval, TimerCallback Callback)
{
    return m_Timer->AddTimer(Interval, Callback);
}

bool QEventLoop::DelTimer(int64_t TimerID)
{
    return m_Timer->DelTimer(TimerID);
}

bool QEventLoop::Dispatch()
{
    while (!m_IsStop)
    {
        long MinTimeOut = m_Timer->GetMinTimeout();
        timeval tv = QTimer::ConvertToTimeval(MinTimeOut);

        g_Log.WriteDebug("Dispatch: min timeout = %ld, tv.sec = %d, tv.usec = %d",
            MinTimeOut, tv.tv_sec, tv.tv_usec);

        m_Backend->Dispatch(tv);
    }

    return true;
}
